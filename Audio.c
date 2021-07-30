#include "Audio.h"

#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include "Common.h"

DECL_ARRAY(CharPtr, char*);

struct Array_CharPtr Audio_GetDevices() {
	struct Array_CharPtr arr = {0};

	if(alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE) {
		const char* res = alcGetString(NULL, ALC_DEVICE_SPECIFIER);

		u32 i = 0;
		while(res[i] == '\0' && res[i + 1] == '\0') {
			Array_CharPtr_PushVal(&arr, res + i);
			i += strlen(res + i) + 1;
		}
	}

	return arr;
}

static bool8 Audio_Initialized = 0;

static ALCdevice* Device   = NULL;
static ALCcontext* Context = NULL;

void Audio_Init(const char* deviceName) {
	if(Audio_Initialized) return;

	Device = alcOpenDevice(deviceName);

	if(!Device) {
		Log(ERROR, "[Audio] Can't open device %s.", (deviceName ? deviceName : "[DEFAULT]"));
		return;
	}

	Context = alcCreateContext(Device, NULL);
	alcMakeContextCurrent(Context);

	if(!alutInitWithoutContext(NULL, NULL)) {
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

void Audio_Quit() {
	if(!Audio_Initialized) return;

	alutExit();
	alcDestroyContext(Context);
	alcCloseDevice(Device);

	Context = NULL;
	Device  = NULL;

	Audio_Initialized = 0;
	Log(INFO, "[Audio] Quit", "");
}

void Audio_SetDistModel(enum Audio_DistModel model) {
	if(!Audio_Initialized) return;
	alDistanceModel(model);
}

//
// Audio_Listener
//

Audio_ListenerProps Audio_Listener_ReadProps() {
	if(!Audio_Initialized) return (Audio_ListenerProps){0};

	Audio_ListenerProps p;
	alGetListenerf(AL_GAIN, &p.MasterGain);
	alGetListenerfv(AL_POSITION, p.Position.d);
	alGetListenerfv(AL_VELOCITY, p.Velocity.d);
	alGetListenerfv(AL_ORIENTATION, p.Orientation.d);
	return p;
}

void Audio_Listener_SetProps(const Audio_ListenerProps* p) {
	if(!Audio_Initialized) return;

	alListenerf(AL_GAIN, p->MasterGain);
	alListenerfv(AL_POSITION, p->Position.d);
	alListenerfv(AL_VELOCITY, p->Velocity.d);
	alListenerfv(AL_ORIENTATION, p->Orientation.d);
}

void Audio_Listener_SyncToCamera(Camera c) {
	if(!Audio_Initialized) return;

	Audio_ListenerProps lp = {
	    .MasterGain  = 1,
	    .Position    = c.Position,
	    .Velocity    = V3(0, 0, 0),
	    .Orientation = {.Forward = Vec3_Norm(Vec3_Sub(c.Position, c.Target)),
	                    .Up      = Vec3_Norm(c.Up)}};
	Audio_Listener_SetProps(&lp);
}

//
// Audio_Buffer
//

Audio_Buffer Audio_Buffer_FromFile(const char* file) {
	Audio_Buffer res = {.Id = 0};

	if(!Audio_Initialized) {
		Log(ERROR, "[Audio] \"%s\" load fail - audio not started.", file);
		return res;
	}

	u32 sz;
	u8* Data = File_ReadToBuffer_Alloc(file, &sz);

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
void Audio_Buffer_Free(Audio_Buffer buf) {
	if(!Audio_Initialized) return;

	if(buf.Id == 0) return;

	alDeleteBuffers(1, &buf.Id);
	ALenum err = alGetError();
	switch(err) {
		case AL_NO_ERROR: break;
		case AL_INVALID_OPERATION:
			Log(ERROR, "[Audio] Buffer #%d free fail - still in use.", buf.Id);
			break;
		case AL_INVALID_NAME:
			Log(ERROR, "[Audio] Buffer #%d free fail - doesn't exist.", buf.Id);
			break;
		default:
			Log(ERROR,
			    "[Audio] Buffer #%d free fail - unknown error (code: %d).",
			    buf.Id,
			    err);
			break;
	}
}

Audio_BufferProps Audio_Buffer_ReadProps(Audio_Buffer buf) {
	if(!Audio_Initialized) return (Audio_BufferProps){0};

	ALint sz, freq, chan, bitD;
	alGetBufferi(buf.Id, AL_SIZE, &sz);
	alGetBufferi(buf.Id, AL_BITS, &bitD);
	alGetBufferi(buf.Id, AL_FREQUENCY, &freq);
	alGetBufferi(buf.Id, AL_CHANNELS, &chan);

	Audio_BufferProps p;
	p.Size        = sz;
	p.BitDepth    = bitD;
	p.Frequency   = freq;
	p.NumChannels = chan;

	p.LenSamples = sz * 8.0 / (chan * bitD);
	p.LenSeconds = p.LenSamples / freq;

	return p;
}

//
// Audio_Source
//

Audio_Source Audio_Source_Init() {
	if(!Audio_Initialized) return (Audio_Source){0};

	Audio_Source s;
	alGenSources(1, &s.Id);
	return s;
}

void Audio_Source_Free(Audio_Source src) {
	if(!Audio_Initialized) return;

	alDeleteSources(1, &src.Id);
}

Audio_SourceProps Audio_Source_ReadProps(Audio_Source src) {
	if(!Audio_Initialized) return (Audio_SourceProps){0};

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

	alGetSourcef(src.Id, AL_SEC_OFFSET, &p.PlayheadSeconds);

	return p;
}

void Audio_Source_SetProps(Audio_Source src, const Audio_SourceProps* p) {
	if(!Audio_Initialized) return;

	alSourcei(src.Id, AL_BUFFER, p->BufferId);

	alSourcef(src.Id, AL_PITCH, p->Pitch);
	alSourcef(src.Id, AL_GAIN, p->Gain);
	alSourcef(src.Id, AL_MIN_GAIN, p->MinGain);
	alSourcef(src.Id, AL_MAX_GAIN, p->MaxGain);

	alSourcei(src.Id, AL_SOURCE_RELATIVE, p->PosRelative);

	alSource3f(src.Id, AL_POSITION, p->Position.x, p->Position.y, p->Position.z);
	alSource3f(src.Id, AL_DIRECTION, p->Direction.x, p->Direction.y, p->Direction.z);
	alSource3f(src.Id, AL_VELOCITY, p->Velocity.x, p->Velocity.y, p->Velocity.z);

	alSourcei(src.Id, AL_LOOPING, p->Loop);

	alSourcef(src.Id, AL_MAX_DISTANCE, p->MaxDistance);
	alSourcef(src.Id, AL_ROLLOFF_FACTOR, p->RolloffFactor);
	alSourcef(src.Id, AL_REFERENCE_DISTANCE, p->HalfVolumeDistance);
}

Audio_Buffer Audio_Source_ReadBuffer(Audio_Source s) {
	if(!Audio_Initialized) return (Audio_Buffer){0};

	ALint buf;
	alGetSourcei(s.Id, AL_BUFFER, &buf);
	return (Audio_Buffer){.Id = buf};
}

void Audio_Source_SetBuffer(Audio_Source s, Audio_Buffer buf) {
	if(!Audio_Initialized) return;

	if(Audio_Source_ReadState(s) == SourceState_Playing) Audio_Source_Stop(s);

	alSourcei(s.Id, AL_BUFFER, buf.Id);
}

r32 Audio_Source_ReadPlayheadPos(Audio_Source src) {
	if(!Audio_Initialized) return 0;

	ALfloat f;
	alGetSourcef(src.Id, AL_SEC_OFFSET, &f);
	return f;
}

void Audio_Source_SeekTo(Audio_Source src, r32 seekSeconds) {
	if(!Audio_Initialized) return;

	alSourcef(src.Id, AL_SEC_OFFSET, seekSeconds);
}

void Audio_Source_SeekBy(Audio_Source src, r32 seekSeconds) {
	if(!Audio_Initialized) return;

	r32 s = Audio_Source_ReadPlayheadPos(src) + seekSeconds;
	alSourcef(src.Id, AL_SEC_OFFSET, s);
}

enum Audio_SourceState Audio_Source_ReadState(Audio_Source src) {
	if(!Audio_Initialized) return SourceState_Invalid;

	ALint s;
	alGetSourcei(src.Id, AL_SOURCE_STATE, &s);
	return s;
}

#define X()                            \
	do {                               \
		if(!Audio_Initialized) return; \
	} while(0)
void Audio_Source_Play(Audio_Source src) {
	X();
	alSourcePlay(src.Id);
}
void Audio_Source_Pause(Audio_Source src) {
	X();
	alSourcePause(src.Id);
}
void Audio_Source_Rewind(Audio_Source src) {
	X();
	alSourceRewind(src.Id);
}
void Audio_Source_Stop(Audio_Source src) {
	X();
	alSourceStop(src.Id);
}
