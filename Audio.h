#ifndef AUDIO_H
#define AUDIO_H

#include "Common.h"
#include "Math3D.h"
#include "Camera.h"

#include <AL/al.h>

/*
 * === AUDIO SYSTEM ===
 *
 * A wrapper around OpenAL and FreeALUT.
 *
 * -- Basic Usage --
 *
 * - Audio_Init(NULL)                 => starts the system
 * - Audio_Listener_SyncToCamera(cam) => moves where the ears are based on the given camera
 * - Audio_Buffer_FromFile(file)      => loads a 16 bit WAV into memory
 * - Audio_Source_Init()              => creates a thing from which that WAV can play
 * - Audio_Source_SetBuffer(src, buf) => tells it to use the WAV
 * - Audio_Source_Play(src)           => makes the source actually play the WAV
 * - Audio_Source_Stop(src)           => makes the source stop playing when you get sick of hearing it.
 * - Audio_Source_Free(src)           => deletes the source, allowing the WAV to be freed
 * - Audio_Buffer_Free(buf)           => deletes the WAV from memory
 * - Audio_Quit()                     => stops the system
 *
 */

DEF_ARRAY(CharPtr, char*);

// Print all audio devices.
Array_CharPtr Audio_GetDevices();

// Start audio system. NULL means default device.
void Audio_Init(const char* Device);

// Quit audio system.
void Audio_Quit();

// How to change volume based on distance from listener
enum Audio_DistModel {
	// Volume doesn't change with distance.
	DistModel_None = AL_NONE,

	// Volume changes inversely with distance.
	DistModel_Inverse = AL_INVERSE_DISTANCE,

	// Volume changes inversely with distance up to a maximum.
	DistModel_InverseClamped = AL_INVERSE_DISTANCE_CLAMPED,

	// Volume changes linearly with distance.
	DistModel_Linear = AL_LINEAR_DISTANCE,

	// Volume changes linearly with distance up to a maximum.
	DistModel_LinearClamped = AL_LINEAR_DISTANCE_CLAMPED,

	// Volume changes exponentially with distance.
	DistModel_Exponential = AL_EXPONENT_DISTANCE,

	// Volume changes exponentially with distance up to a maximum.
	DistModel_ExponentialClamped = AL_EXPONENT_DISTANCE_CLAMPED,

	// Default value
	DistModel_Default = DistModel_InverseClamped,
};

// Set how volume changes with distance.
void Audio_SetDistModel(enum Audio_DistModel model);

//
// Audio_Listener
//

typedef struct Audio_ListenerProps Audio_ListenerProps;

struct Audio_ListenerProps {
	r32 MasterGain; // How loud will all audio be?

	Vec3 Position; // Where the listener is
	Vec3 Velocity; // How fast the listener is moving

	union {
		r32 d[6]; // Orientation as a six float array
		struct {
			Vec3 Forward, Up;
		};
	} Orientation; // Two vectors, pointing forward and upward from the camera
};

// Get the current properties of the listener
Audio_ListenerProps Audio_Listener_ReadProps();

// Set the properties of the listener
void Audio_Listener_SetProps(const Audio_ListenerProps*);

// Make the listener follow a given camera
void Audio_Listener_SyncToCamera(Camera);

//
// Audio_Buffer
//

typedef struct Audio_BufferProps Audio_BufferProps;
typedef struct Audio_Buffer Audio_Buffer;

// A sound or music's properties.
struct Audio_BufferProps {
	u32 Size;        // Size in bytes
	u32 BitDepth;    // Bit depth
	u32 Frequency;   // Frequency in Hz
	u32 NumChannels; // How many channels, e.g. 1 = mono, 2 = stereo

	r32 LenSamples; // Length of buffer in samples
	r32 LenSeconds; // Duration of buffer in seconds
};

// A sound or music.
struct Audio_Buffer {
	u32 Id;
};

// Create a buffer from a file.
// @warn The file MUST be a 16 bit WAV.
Audio_Buffer Audio_Buffer_FromFile(const char* file);

// Delete a buffer from memory
void Audio_Buffer_Free(Audio_Buffer buf);

// Get the properties of a given buffer
Audio_BufferProps Audio_Buffer_ReadProps(Audio_Buffer buf);

//
// Audio_Source
//

enum Audio_SourceState {
	// There isn't even a valid source.
	SourceState_Invalid = -1,

	// The source is waiting at 00:00.
	SourceState_Initial = AL_INITIAL,

	// The source is currently playing audio.
	SourceState_Playing = AL_PLAYING,

	// The source has been paused in the middle of playing.
	SourceState_Paused = AL_PAUSED,

	// The source is waiting at the end of its buffer.
	SourceState_Stopped = AL_STOPPED
};

typedef struct Audio_SourceProps Audio_SourceProps;
typedef struct Audio_Source Audio_Source;

// Properties of an audio source.
struct Audio_SourceProps {
	enum Audio_SourceState State; // Is the source playing, paused, etc.

	u32 BufferId; // Buffer, which is currently attached to the source.

	r32 Pitch; // How squeaky or bassy the source sounds
	r32 Gain;  // Loudness of the source

	r32 MinGain; // Minimum volume, the quietest the source can be
	r32 MaxGain; // Maximum volume, the loudest the source can be

	bool8 PosRelative; // Is the position relative to the listener or absolute in the world?
	Vec3 Position;     // Where the source is
	Vec3 Velocity;     // How fast the source is moving
	Vec3 Direction;    // Where the source is looking

	bool8 Loop; // Does the audio loop after reaching the end?

	r32 MaxDistance;   // (Used by linear distance models) Distance at which volume becomes 0
	r32 RolloffFactor; // How much distance affects volume.
	r32 HalfVolumeDistance; // Distance at which the source should be heard at half volume

	r32 PlayheadSeconds; // How far into its buffer the source has reached (not set by SetProps)
};

// A thing emitting audio somewhere.
struct Audio_Source {
	u32 Id;
};

// Create a new audio source
Audio_Source Audio_Source_Init();

// Delete an audio source
void Audio_Source_Free(Audio_Source);

// Update the properties of a source
void Audio_Source_SetProps(Audio_Source, const Audio_SourceProps*);

// Get the properties of a source
Audio_SourceProps Audio_Source_ReadProps(Audio_Source);

// Set a source's buffer and rewind the source
void Audio_Source_SetBuffer(Audio_Source, Audio_Buffer buf);

// Make the source go to a given position in the audio
void Audio_Source_SeekTo(Audio_Source, r32 seekSeconds);

// Jump the source's audio a given number of seconds forwards or backwards
void Audio_Source_SeekBy(Audio_Source, r32 seekSeconds);

// Get which buffer is bound to a source
Audio_Buffer Audio_Source_ReadBuffer(Audio_Source);

// Get how many seconds of the audio the source has played
r32 Audio_Source_ReadPlayheadPos(Audio_Source);

// Get the state of the source
enum Audio_SourceState Audio_Source_ReadState(Audio_Source);

// Play the source from the beginning or from where it was paused
void Audio_Source_Play(Audio_Source src);

// Pause the source at its current location
void Audio_Source_Pause(Audio_Source src);

// Pause the source and seek to 00:00
void Audio_Source_Rewind(Audio_Source src);

// Stop the source and seek to the end of the audio
void Audio_Source_Stop(Audio_Source src);

#endif
