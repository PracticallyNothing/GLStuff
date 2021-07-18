#ifndef RES_H
#define RES_H

#include "Common.h"
#include "Math3D.h"

//
// Threaded resource loading.
//

void Res_Init();
void Res_LoadMesh(const char* file);
void Res_LoadAudio(const char* file);
void Res_LoadScene(const char* file);

#endif
