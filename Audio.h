#ifndef AUDIO_H
#define AUDIO_H

#include "Common.h"
#include "Math3D.h"
#include "Camera.h"

#include <AL/al.h>

DEF_ARRAY(CharPtr, char *);
struct Array_CharPtr Audio_GetDevices();

/// Initialize the audio system. A call with NULL makes it guess the best device.
void Audio_Init(const char* Device);
void Audio_Quit();

enum Audio_DistModel {
	DistModel_None               = AL_NONE,                      // Volume doesn't change with distance.
	DistModel_Inverse            = AL_INVERSE_DISTANCE,          // Volume changes inversely with distance.
	DistModel_InverseClamped     = AL_INVERSE_DISTANCE_CLAMPED,  // Volume changes inversely with distance up to a maximum.
	DistModel_Linear             = AL_LINEAR_DISTANCE,           // Volume changes linearly with distance.
	DistModel_LinearClamped      = AL_LINEAR_DISTANCE_CLAMPED,   // Volume changes linearly with distance up to a maximum.
	DistModel_Exponential        = AL_EXPONENT_DISTANCE,         // Volume changes exponentially with distance.
	DistModel_ExponentialClamped = AL_EXPONENT_DISTANCE_CLAMPED, // Volume changes exponentially with distance up to a maximum.

	DistModel_Default = DistModel_InverseClamped, // Default value.
};
void Audio_SetDistModel(enum Audio_DistModel model);

//
// Audio_Listener
//

typedef struct Audio_ListenerProps Audio_ListenerProps;

struct Audio_ListenerProps {
	r32 MasterGain;

	Vec3 Position;
	Vec3 Velocity;

	union {
		r32 d[6];
		struct { Vec3 Forward, Up; };
	} Orientation;
};

Audio_ListenerProps Audio_Listener_ReadProps();
void                Audio_Listener_SetProps(const Audio_ListenerProps* props);
void                Audio_Listener_SyncToCamera(Camera cam);

//
// Audio_Buffer
//

typedef struct Audio_BufferProps Audio_BufferProps;
typedef struct Audio_Buffer      Audio_Buffer;

struct Audio_BufferProps {
	u32 Size;
	u32 BitDepth;
	u32 Frequency;
	u32 NumChannels;

	r32 LenSamples;
	r32 LenSeconds;
};

struct Audio_Buffer { 
	u32 Id; 
};

Audio_Buffer      Audio_Buffer_FromFile(const char* file);
void              Audio_Buffer_Free(Audio_Buffer buf);
Audio_BufferProps Audio_Buffer_ReadProps(Audio_Buffer buf);

//
// Audio_Source
//

enum Audio_SrcState {
	SrcState_Invalid = -1,
	SrcState_Initial = AL_INITIAL,
	SrcState_Playing = AL_PLAYING,
	SrcState_Paused  = AL_PAUSED,
	SrcState_Stopped = AL_STOPPED
};

typedef struct Audio_SrcProps Audio_SrcProps;
typedef struct Audio_Src      Audio_Src;

struct Audio_SrcProps {
	enum Audio_SrcState State;

	u32 BufferId;

	r32 Pitch;
	r32 Gain;

	r32 MinGain;
	r32 MaxGain;

	bool8 PosRelative;
	Vec3 Position;
	Vec3 Velocity;
	Vec3 Direction;

	bool8 Loop;

	r32 MaxDistance;
	r32 RolloffFactor;
	r32 HalfVolumeDistance;

	r32 PlayheadSeconds; // Position of the play head in seconds.
};

struct Audio_Src { u32 Id; }; // A thing emitting audio somewhere.

Audio_Src Audio_Src_Init();
void      Audio_Src_Free(Audio_Src src);

Audio_Buffer Audio_Src_ReadBuffer(Audio_Src s);
void         Audio_Src_SetBuffer(Audio_Src s, Audio_Buffer buf);

Audio_SrcProps Audio_Src_ReadProps(Audio_Src src); 
void           Audio_Src_SetProps(Audio_Src src, const Audio_SrcProps* props);  

r32  Audio_Src_ReadPlayheadPos(Audio_Src src);
void Audio_Src_SeekTo(Audio_Src src, r32 seekSeconds);
void Audio_Src_SeekBy(Audio_Src src, r32 seekSeconds);

enum Audio_SrcState Audio_Src_ReadState(Audio_Src src);

void Audio_Src_Play  (Audio_Src src);
void Audio_Src_Pause (Audio_Src src);
void Audio_Src_Rewind(Audio_Src src);
void Audio_Src_Stop  (Audio_Src src);

#endif
