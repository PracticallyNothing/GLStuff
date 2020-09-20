#include "Audio.h"

#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>

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

void Audio_Init(const char* device)
{
	ALCdevice *dev = alcOpenDevice(device);

	if(!dev) {
		Log(ERROR, "OpenAL couldn't open audio device \"%s\".", (device ? device : "[DEFAULT]"));
		return;
	}

	ALCcontext *ctx = alcCreateContext(dev, NULL);
	alcMakeContextCurrent(ctx);
}
