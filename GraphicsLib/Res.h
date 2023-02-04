#ifndef RES_H
#define RES_H

#include "Common.h"
#include "Render.h"

/// Is the texture with the given filename residing somewhere in memory already?
bool8 Res_Texture_InCache(const char* filename);

/// Load a texture from disk or get it from the cache if it's there.
Texture* Res_Texture_GetOrLoad(const char* filename);

#endif
