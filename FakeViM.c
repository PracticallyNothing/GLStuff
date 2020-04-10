#include "FakeViM.h"

#include "Render.h"


#define WHITE V4C(1, 1, 1, 1)
#define BLACK V4C(0, 0, 0, 1)
#define RED V4C(1, 0, 0, 1)
#define GREEN V4C(0, 1, 0, 1)
#define BLUE V4C(0, 0, 1, 1)
#define CYAN V4C(0, 1, 1, 1)
#define YELLOW V4C(1, 1, 0, 1)
#define ORANGE V4C(0.8, 0.4, 0.2, 1)

enum Mode {
	Mode_Normal,
	Mode_Command,
	Mode_Insert,
	Mode_Replace,
	Mode_Visual,
	Mode_VisualLine,
	Mode_VisualBlock
} Mode;

static const char *ModeStrings[] = {
    "NORMAL", "INSERT", "REPLACE", "VISUAL", "VISUAL (line)", "VISUAL (block)"};
static const RGBA ModeFgColors[] = {BLACK, BLUE, WHITE, BLACK, BLACK, BLACK};
static const RGBA ModeBgColors[] = {YELLOW, CYAN, RED, ORANGE, ORANGE, ORANGE};

void FakeVIM_Input(SDL_Event *e) {
	switch(e->type) {
		case SDL_KEYUP:
			switch(e->key.keysym.sym) {
				case SDLK_ESCAPE: Mode = Mode_Normal; break;
				case SDLK_i: {
				} break;
				case SDLK_a: Mode = Mode_Insert; break;
				case SDLK_r: Mode = Mode_Replace; break;
				case SDLK_v: {
					if(e->key.keysym.mod & KMOD_SHIFT)
						Mode = Mode_VisualLine;
					else if(e->key.keysym.mod & KMOD_CTRL)
						Mode = Mode_VisualBlock;
					else
						Mode = Mode_Visual;
				} break;
			}
	}
}

void FakeVIM_Render() {
	Vec2 screenSz;
	{
		RSys_Size _sz = RSys_GetSize();
		screenSz = V2(_sz.Width, _sz.Height);
	}
	Vec2 textSz =
	    R2D_GetTextExtents(&R2D_DefaultFont, " %s ", ModeStrings[Mode]);

	R2D_DrawText(V2(0, screenSz.y - textSz.y), ModeFgColors[Mode],
	             ModeBgColors[Mode], &R2D_DefaultFont, " %s ",
	             ModeStrings[Mode]);
}
