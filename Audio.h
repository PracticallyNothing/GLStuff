#ifndef AUDIO_H
#define AUDIO_H

#include "Common.h"

DEF_ARRAY(CharPtr, char *);
extern struct Array_CharPtr Audio_GetDevices();

extern void Audio_Init(const char* Device);
extern void Audio_PlaySound();
extern void Audio_PlayMusic();

#endif
