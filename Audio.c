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

static bool8 Audio_Initialized = 0;

static ALCdevice  *Device = NULL;
static ALCcontext *Context = NULL;

void Audio_Init(const char *deviceName) {
	if(Audio_Initialized) return;

	Device = alcOpenDevice(deviceName);

	if(!Device) {
		Log(ERROR, "[Audio] Can't open device %s.", (deviceName ? deviceName : "[DEFAULT]"));
		return;
	}

	Context = alcCreateContext(Device, NULL);
	alcMakeContextCurrent(Context);

	if(!alutInitWithoutContext(NULL, NULL))
	{
		Log(ERROR, "[Audio] ALUT init fail: %s.", alutGetErrorString(alutGetError()));
		return;
	}

	Log(INFO, "[Audio] Started", "");
	Audio_Initialized = 1;
	alDistanceModel(DistModel_Default);

	// Clear error state.
	alGetError();
	alutGetError();
}

void Audio_Quit()
{
	if(!Audio_Initialized)
		return;

	alutExit();
	alcDestroyContext(Context);
	alcCloseDevice(Device);

	Context = NULL;
	Device = NULL;

	Audio_Initialized = 0;
	Log(INFO, "[Audio] Quit", "");
}

void Audio_SetDistModel(enum Audio_DistModel model)
{
	if(!Audio_Initialized)
		return;
	alDistanceModel(model);
}

//
// Audio_Listener
//

Audio_ListenerProps 
Audio_Listener_ReadProps()
{
	if(!Audio_Initialized)
		return (Audio_ListenerProps){0};

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
	if(!Audio_Initialized)
		return;

	alListenerf(AL_GAIN, p->MasterGain);
	alListenerfv(AL_POSITION, p->Position.d);
	alListenerfv(AL_VELOCITY, p->Velocity.d);
	alListenerfv(AL_ORIENTATION, p->Orientation.d);
}

void
Audio_Listener_SyncToCamera(Camera c)
{
	if(!Audio_Initialized)
		return;

	Audio_ListenerProps lp = {
		.MasterGain  = 1,
		.Position    = c.Position,
		.Velocity    = V3(0, 0, 0),
		.Orientation = {
			.Forward = Vec3_Norm(Vec3_Sub(c.Position, c.Target)),
			.Up = Vec3_Norm(c.Up)
		}
	};
	Audio_Listener_SetProps(&lp);
}

//
// Audio_Buffer
//

Audio_Buffer
Audio_Buffer_FromFile(const char* file)
{
	Audio_Buffer res = { .Id = 0 };

	if(!Audio_Initialized) {
		Log(ERROR, "[Audio] \"%s\" load fail - audio not started.", file);
		return res;
	}

	u32 sz;
	u8 *Data = File_ReadToBuffer_Alloc(file, &sz);

	if(!Data) {
		Log(ERROR, "[Audio] %s load fail - file not found.", file);
		return res;
	}

	res.Id = alutCreateBufferFromFileImage(Data, sz);
	if(!res.Id) {
		ALenum err = alutGetError();
		Log(ERROR, "[Audio] %s load fail - ALUT error: %s.", file, alutGetErrorString(err));
	}

	return res;
}
void
Audio_Buffer_Free(Audio_Buffer buf)
{
	if(!Audio_Initialized)
		return;

	if(buf.Id == 0)
		return;

	alDeleteBuffers(1, &buf.Id);
	ALenum err = alGetError();
	switch(err)
	{
		case AL_NO_ERROR: 
			break;
		case AL_INVALID_OPERATION: 
			Log(ERROR, "[Audio] Buffer #%d free fail - still in use.", buf.Id);
			break;
		case AL_INVALID_NAME:
			Log(ERROR, "[Audio] Buffer #%d free fail - doesn't exist.", buf.Id);
			break;
		default:
			Log(ERROR, "[Audio] Buffer #%d free fail - unknown error (code: %d).", buf.Id, err);
			break;
	}
}

Audio_BufferProps 
Audio_Buffer_ReadProps(Audio_Buffer buf)
{
	if(!Audio_Initialized)
		return (Audio_BufferProps){0};

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

Audio_Src
Audio_Src_Init()
{
	if(!Audio_Initialized)
		return (Audio_Src){0};

	Audio_Src s;
	alGenSources(1, &s.Id);
	return s;
}

void Audio_Src_Free(Audio_Src src)
{
	if(!Audio_Initialized)
		return;

	alDeleteSources(1, &src.Id);
}

Audio_SrcProps 
Audio_Src_ReadProps(Audio_Src src)
{
	if(!Audio_Initialized)
		return (Audio_SrcProps){0};

	Audio_SrcProps p;
	p.State = Audio_Src_ReadState(src);

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

	alGetSourcef(src.Id, AL_SEC_OFFSET, &p.PlayheadSeconds);

	return p;
}

void
Audio_Src_SetProps(Audio_Src src, const Audio_SrcProps* p)
{
	if(!Audio_Initialized)
		return;

	alSourcei(src.Id, AL_BUFFER, p->BufferId);

	alSourcef(src.Id, AL_PITCH,    p->Pitch);
	alSourcef(src.Id, AL_GAIN,     p->Gain);
	alSourcef(src.Id, AL_MIN_GAIN, p->MinGain);
	alSourcef(src.Id, AL_MAX_GAIN, p->MaxGain);

	alSourcei(src.Id, AL_SOURCE_RELATIVE, p->PosRelative);

	alSource3f(src.Id, AL_POSITION,  p->Position.x, p->Position.y, p->Position.z);
	alSource3f(src.Id, AL_DIRECTION, p->Direction.x, p->Direction.y, p->Direction.z);
	alSource3f(src.Id, AL_VELOCITY,  p->Velocity.x, p->Velocity.y, p->Velocity.z);

	alSourcei(src.Id, AL_LOOPING, p->Loop);

	alSourcef(src.Id, AL_MAX_DISTANCE,       p->MaxDistance);
	alSourcef(src.Id, AL_ROLLOFF_FACTOR,     p->RolloffFactor);
	alSourcef(src.Id, AL_REFERENCE_DISTANCE, p->HalfVolumeDistance);
}


Audio_Buffer
Audio_Src_ReadBuffer(Audio_Src s)
{
	if(!Audio_Initialized)
		return (Audio_Buffer){0};
	
	ALint buf;
	alGetSourcei(s.Id, AL_BUFFER, &buf);
	return (Audio_Buffer) { .Id = buf };
}

void
Audio_Src_SetBuffer(Audio_Src s, Audio_Buffer buf)
{
	if(!Audio_Initialized)
		return;

	if(Audio_Src_ReadState(s) == SrcState_Playing)
		Audio_Src_Stop(s);

	alSourcei(s.Id, AL_BUFFER, buf.Id);
}

r32 
Audio_Src_ReadPlayheadPos(Audio_Src src)
{
	if(!Audio_Initialized)
		return 0;

	ALfloat f;
	alGetSourcef(src.Id, AL_SEC_OFFSET, &f);
	return f;
}

void
Audio_Src_SeekTo(Audio_Src src, r32 seekSeconds)
{
	if(!Audio_Initialized)
		return;

	alSourcef(src.Id, AL_SEC_OFFSET, seekSeconds);
}

void
Audio_Src_SeekBy(Audio_Src src, r32 seekSeconds)
{
	if(!Audio_Initialized)
		return;

	r32 s = Audio_Src_ReadPlayheadPos(src) + seekSeconds;
	alSourcef(src.Id, AL_SEC_OFFSET, s);
}

enum Audio_SrcState 
Audio_Src_ReadState(Audio_Src src)
{
	if(!Audio_Initialized)
		return SrcState_Invalid;

	ALint s;
	alGetSourcei(src.Id, AL_SOURCE_STATE, &s);
	return s;
}

#define X() do { if(!Audio_Initialized) return; } while(0)
void Audio_Src_Play  (Audio_Src src) { X(); alSourcePlay  (src.Id); }
void Audio_Src_Pause (Audio_Src src) { X(); alSourcePause (src.Id); }
void Audio_Src_Rewind(Audio_Src src) { X(); alSourceRewind(src.Id); }
void Audio_Src_Stop  (Audio_Src src) { X(); alSourceStop  (src.Id); }
