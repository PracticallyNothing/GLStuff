#include "Audio.h"

#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include "Common.h"

DECL_ARRAY(CharPtr, char *);

struct Array_CharPtr Audio_GetDevices() {
	struct Array_CharPtr arr = {0};

	if(alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE)
	{
		const char *res = alcGetString(NULL, ALC_DEVICE_SPECIFIER);

		u32 i = 0;
		while(res[i] == '\0' && res[i+1] == '\0') {
			Array_CharPtr_PushVal(&arr, res+i);
			i += strlen(res+i) + 1;
		}
	}

	return arr;
}

static ALCdevice  *Device = NULL;
static ALCcontext *Context = NULL;

void Audio_Init(const char *deviceName) {
	if(Device) {
		Log(WARN, "Attempt to initialize audio system, but it's already initialized.", "");
		return;
	}

	Device = alcOpenDevice(deviceName);

	if(!Device) {
		Log(ERROR, "OpenAL couldn't open audio device \"%s\".", (deviceName ? deviceName : "[DEFAULT]"));
		return;
	}

	Context = alcCreateContext(Device, NULL);
	alcMakeContextCurrent(Context);

	if(!alutInitWithoutContext(NULL, NULL))
	{
		Log(ERROR, 
				"Failed to initialize ALUT.\n"
				"ALUT reports the following error: %s", 
				alutGetErrorString(alutGetError()));
		return;
	}

	alDistanceModel(Audio_DistModel_Inverse);
}

void Audio_Quit()
{
	alutExit();
	alcDestroyContext(Context);
	alcCloseDevice(Device);

	Context = NULL;
	Device = NULL;
}

void Audio_SetDistModel(enum Audio_DistModel model)
{
	alDistanceModel(model);
}

//
// Audio_Listener
//

Audio_ListenerProps 
Audio_Listener_ReadProps()
{
	Audio_ListenerProps p;
	alGetListenerf(AL_GAIN, &p.MasterGain);
	alGetListenerfv(AL_POSITION, p.Position.d);
	alGetListenerfv(AL_VELOCITY, p.Velocity.d);
	alGetListenerfv(AL_ORIENTATION, p.Orientation.d);
	return p;
}

void 
Audio_Listener_SetProps(const Audio_ListenerProps* p)
{
	alListenerf(AL_GAIN, p->MasterGain);
	alListenerfv(AL_POSITION, p->Position.d);
	alListenerfv(AL_VELOCITY, p->Velocity.d);
	alListenerfv(AL_ORIENTATION, p->Orientation.d);
}

//
// Audio_Buffer
//

Audio_Buffer
Audio_Buffer_FromFile(const char* file)
{
	u32 sz;
	u8 *Data = File_ReadToBuffer_Alloc(file, &sz);

	Audio_Buffer res = { .Id = 0 };

	if(Data) {
		alutGetError(); // Clear any errors.

		res.Id = alutCreateBufferFromFileImage(Data, sz);
		if(!res.Id) {
			ALenum err = alutGetError();
			Log(ERROR, 
				"Failed to load audio buffer from file \"%s\".\n"
				"ALUT reports the following error: %s", 
				file, alutGetErrorString(err));
		}
	}

	return res;
}
void
Audio_Buffer_Free(Audio_Buffer buf)
{
	if(buf.Id == 0)
		return;

	alGetError();

	alDeleteBuffers(1, &buf.Id);
	ALenum err = alGetError();
	switch(err)
	{
		case AL_INVALID_OPERATION: 
			Log(ERROR, 
				"(AL_INVALID_OPERATION: %d) Buffer #%d is still in use and cannot be freed yet.",
				AL_INVALID_OPERATION, buf.Id);
			break;
		case AL_INVALID_NAME:
			Log(ERROR,
				"(AL_INVALID_NAME: %d) Buffer #%d doesn't exist and cannot be freed yet.",
				AL_INVALID_NAME, buf.Id);
			break;
		case AL_NO_ERROR: break;
		default:
			Log(ERROR, 
				"(Unknown error: %d) An unknown error occured during buffer #%d's deletion.",
				err, buf.Id);
			break;
	}
}

Audio_BufferProps 
Audio_Buffer_ReadProps(Audio_Buffer buf)
{
	ALint sz, freq, chan, bitD;
	alGetBufferi(buf.Id, AL_SIZE, &sz);
	alGetBufferi(buf.Id, AL_BITS, &bitD);
	alGetBufferi(buf.Id, AL_FREQUENCY, &freq);
	alGetBufferi(buf.Id, AL_CHANNELS, &chan);

	Audio_BufferProps p;
	p.Size = sz;
	p.BitDepth = bitD;
	p.Frequency = freq;
	p.NumChannels = chan;

	p.LenSamples = sz * 8.0 / (chan * bitD);
	p.LenSeconds = p.LenSamples / freq;

	return p;
}

//
// Audio_Source
//

Audio_Source
Audio_Source_Init()
{
	Audio_Source s;
	alGenSources(1, &s.Id);
	return s;
}

void Audio_Source_Free(Audio_Source src)
{
	alDeleteSources(1, &src.Id);
}

Audio_SourceProps 
Audio_Source_ReadProps(Audio_Source src)
{
	Audio_SourceProps p;
	p.State = Audio_Source_ReadState(src);

	ALint buf;
	alGetSourcei(src.Id, AL_BUFFER, &buf);
	p.BufferId = buf;

	alGetSourcef(src.Id, AL_PITCH, &p.Pitch);
	alGetSourcef(src.Id, AL_GAIN, &p.Gain);
	alGetSourcef(src.Id, AL_MIN_GAIN, &p.MinGain);
	alGetSourcef(src.Id, AL_MAX_GAIN, &p.MaxGain);

	ALint pRel;
	alGetSourcei(src.Id, AL_SOURCE_RELATIVE, &pRel);
	p.PosRelative = pRel;

	alGetSourcefv(src.Id, AL_POSITION, p.Position.d);
	alGetSourcefv(src.Id, AL_DIRECTION, p.Direction.d);
	alGetSourcefv(src.Id, AL_VELOCITY, p.Velocity.d);

	ALint loop;
	alGetSourcei(src.Id, AL_LOOPING, &loop);
	p.Loop = loop;

	alGetSourcef(src.Id, AL_MAX_DISTANCE, &p.MaxDistance);
	alGetSourcef(src.Id, AL_ROLLOFF_FACTOR, &p.RolloffFactor);
	alGetSourcef(src.Id, AL_REFERENCE_DISTANCE, &p.HalfVolumeDistance);

	alGetSourcef(src.Id, AL_SEC_OFFSET, &p.PlayHeadSeconds);

	return p;
}

void
Audio_Source_SetProps(Audio_Source src, const Audio_SourceProps* p)
{
	alSourcei(src.Id, AL_BUFFER, p->BufferId);

	alSourcef(src.Id, AL_PITCH,    p->Pitch);
	alSourcef(src.Id, AL_GAIN,     p->Gain);
	alSourcef(src.Id, AL_MIN_GAIN, p->MinGain);
	alSourcef(src.Id, AL_MAX_GAIN, p->MaxGain);

	alSourcei(src.Id, AL_SOURCE_RELATIVE, p->PosRelative);

	alSourcefv(src.Id, AL_POSITION,  p->Position.d);
	alSourcefv(src.Id, AL_DIRECTION, p->Direction.d);
	alSourcefv(src.Id, AL_VELOCITY,  p->Velocity.d);

	alSourcei(src.Id, AL_LOOPING, p->Loop);

	alSourcef(src.Id, AL_MAX_DISTANCE,       p->MaxDistance);
	alSourcef(src.Id, AL_ROLLOFF_FACTOR,     p->RolloffFactor);
	alSourcef(src.Id, AL_REFERENCE_DISTANCE, p->HalfVolumeDistance);
}


Audio_Buffer
Audio_Source_ReadBuffer(Audio_Source s)
{
	ALint buf;
	alGetSourcei(s.Id, AL_BUFFER, &buf);
	return (Audio_Buffer) { .Id = buf };
}

void
Audio_Source_SetBuffer(Audio_Source s, Audio_Buffer buf)
{
	alSourcei(s.Id, AL_BUFFER, buf.Id);
}

r32 
Audio_Source_ReadPlayHeadPos(Audio_Source src)
{
	r32 f;
	alGetSourcef(src.Id, AL_SEC_OFFSET, &f);
	return f;
}

void
Audio_Source_Seek(Audio_Source src, r32 seekSeconds)
{
	alSourcef(src.Id, AL_SEC_OFFSET, seekSeconds);
}

enum Audio_SourceState 
Audio_Source_ReadState(Audio_Source src)
{
	i32 s;
	alGetSourcei(src.Id, AL_SOURCE_STATE, &s);
	return s;
}

void Audio_Source_Play(Audio_Source src)   { alSourcePlay(src.Id); }
void Audio_Source_Pause(Audio_Source src)  { alSourcePause(src.Id); }
void Audio_Source_Rewind(Audio_Source src) { alSourceRewind(src.Id); }
void Audio_Source_Stop(Audio_Source src)   { alSourceStop(src.Id); }
