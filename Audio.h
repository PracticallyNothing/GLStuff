#ifndef AUDIO_H
#define AUDIO_H

#include "Common.h"
#include "Math3D.h"

#include <AL/al.h>

DEF_ARRAY(CharPtr, char *);
extern struct Array_CharPtr Audio_GetDevices();

/// Initialize the audio system. A call with NULL makes it guess the best device.
extern void Audio_Init(const char* Device);
extern void Audio_Quit();

enum Audio_DistModel {
	/// Audio will not get attenuated with distance.
	Audio_DistModel_None = AL_NONE,
	/// Volume will realistically lower with the inverse of the distance,
	/// but won't disappear beyond the max distance of the AL::Source.
	Audio_DistModel_Inverse = AL_INVERSE_DISTANCE,
	/// Default value: Volume realistically lowers with the inverse of the distance
	/// and will not go higher than the original level,
	/// but won't disappear beyond the max distance of the source.
	Audio_DistModel_InverseClamped = AL_INVERSE_DISTANCE_CLAMPED,
	/// Volume lowers with distance, disappearing beyond the source's max distance.
	Audio_DistModel_Linear = AL_LINEAR_DISTANCE,
	/// Volume lowers with distance and will not go higher than the original level,
	/// disappearing beyond the source's max distance.
	Audio_DistModel_LinearClamped = AL_LINEAR_DISTANCE_CLAMPED,
	/// Volume will exponentially decrease with the power of the source's roloff factor.
	Audio_DistModel_Exponential = AL_EXPONENT_DISTANCE,
	/// Volume will exponentially decrease with the power of the source's roloff factor
	/// and will not go higher than the original level.
	Audio_DistModel_ExponentialClamped = AL_EXPONENT_DISTANCE_CLAMPED,
};
extern void Audio_SetDistModel(enum Audio_DistModel model);

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

extern Audio_ListenerProps Audio_Listener_ReadProps();
extern void                Audio_Listener_SetProps(const Audio_ListenerProps* props);

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

extern Audio_Buffer      Audio_Buffer_FromFile(const char* file);
extern void              Audio_Buffer_Free(Audio_Buffer buf);
extern Audio_BufferProps Audio_Buffer_ReadProps(Audio_Buffer buf);

//
// Audio_Source
//

enum Audio_SourceState {
	SourceState_Initial = AL_INITIAL,
	SourceState_Playing = AL_PLAYING,
	SourceState_Paused = AL_PAUSED,
	SourceState_Stopped = AL_STOPPED
};

typedef struct Audio_SourceProps Audio_SourceProps;
typedef struct Audio_Source      Audio_Source;

struct Audio_SourceProps {
	enum Audio_SourceState State;

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

	/// Position of the play head in seconds.
	r32 PlayHeadSeconds;
};

struct Audio_Source { 
	u32 Id;
};
extern Audio_Source Audio_Source_Init();
extern void         Audio_Source_Free(Audio_Source src);

extern Audio_Buffer Audio_Source_ReadBuffer(Audio_Source s);
extern void         Audio_Source_SetBuffer(Audio_Source s, Audio_Buffer buf);

extern Audio_SourceProps Audio_Source_ReadProps(Audio_Source src); 
extern void              Audio_Source_SetProps(Audio_Source src, const Audio_SourceProps* props);  

extern r32  Audio_Source_ReadPlayHeadPos(Audio_Source src);
extern void Audio_Source_SeekTo(Audio_Source src, r32 seekSeconds);
extern void Audio_Source_SeekBy(Audio_Source src, r32 seekSeconds);

extern enum Audio_SourceState Audio_Source_ReadState(Audio_Source src);

extern void Audio_Source_Play(Audio_Source src);
extern void Audio_Source_Pause(Audio_Source src);
extern void Audio_Source_Rewind(Audio_Source src);
extern void Audio_Source_Stop(Audio_Source src);

#endif
