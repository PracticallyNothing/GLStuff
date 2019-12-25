#ifndef EDITOR_H
#define EDITOR_H

#include <SDL2/SDL_events.h>

void Editor_Init();
void Editor_HandleInput(SDL_Event *e);
void Editor_Render();

#endif
