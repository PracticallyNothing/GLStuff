#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "Math3D.h"
#include "Render.h"
#include "SDL_keycode.h"
#include "Shader.h"
#include "WavefrontOBJ.h"
#include "stb_image.h"

const Vec2 HexGrid_Pos = V2C(-57/1920.0, 425 / 1080.0);
const Vec2 HexGrid_Size = V2C(1885 / 1920.0, 644 / 1080.0);
const Vec2 HexGrid_TileSize = V2C(243 / 1920.0, 147 / 1080.0);
const r32  HexGrid_TileColumnYOffset = (147 / 1080.0) / 2;

const r32 HexGrid_TileCenterXOffset = 69/1920.0;
const Vec2 HexGrid_TileCenterSize = V2C(106/1920.0, 147/1080.0);

Vec2 HexGrid_GetTilePos(i32 tile)
{
	const r32 xColSize = (HexGrid_TileSize.x + HexGrid_TileCenterSize.x) / 2;

	u32 row = tile % 4, 
		col = tile / 4;

	u32 w = RSys_GetSize().Width,
		h = RSys_GetSize().Height;

	r32 yOffset = (col % 2 ? HexGrid_TileColumnYOffset : 0) * h - (col % 2 ? 2 : 0); 

	return Vec2_Add(
		V2(col * (xColSize * HexGrid_Size.x * w - 1.5), 
		   row * HexGrid_TileSize.y * h + yOffset - row * 5), 
		Vec2_MultVec(HexGrid_Pos, V2(w, h))
	); 
}

i32 HexGrid_SelectTile(Vec2 Pos) {
	Pos.x /= RSys_GetSize().Width;
	Pos.y /= RSys_GetSize().Height;

	if(Pos.x < HexGrid_Pos.x ||
	   Pos.x > HexGrid_Pos.x + HexGrid_Size.x ||
	   Pos.y < HexGrid_Pos.y ||
	   Pos.y > HexGrid_Pos.y + HexGrid_Size.y)
		return -1;

	// What is considered a column here is the left triangle of the hexagon plus its rectangular center.
	r32 xColSize = (HexGrid_TileSize.x + HexGrid_TileCenterSize.x) / 2;
	r32 xx = fmod(Pos.x, xColSize);
	i32 xCol = Pos.x / xColSize;

	// Case 1: The point is in the rectangular middle area of a hexagon column.
	if(xx <= HexGrid_TileCenterSize.x)
	{
		r32 yy = Pos.y - HexGrid_Pos.y - (xCol % 2 ? HexGrid_TileColumnYOffset : 0);
		// Early exit if the point is above or below the tiles.
		if(yy < 0 || yy > HexGrid_TileSize.y * 4)
			return -1;
		return xCol * 4 + yy / HexGrid_TileSize.y;
	}
	// Case 2: The point is in one of the right triangles of the hexagons.
	else {
		r32 len = HexGrid_TileSize.x - HexGrid_TileCenterSize.x;
		r32 xNorm = (xx - HexGrid_TileCenterSize.x) / len * 2;

		r32 leftTriHeight  = (1.0 - xNorm) * HexGrid_TileSize.y;
		r32 rightTriHeight =        xNorm  * HexGrid_TileSize.y;

		r32 yy = Pos.y - HexGrid_Pos.y;

		     if(!xCol % 2 && yy < rightTriHeight / 2) return -1;
		else if( xCol % 2 && yy <  leftTriHeight / 2) return -1;

		i32 yRow = yy / HexGrid_TileSize.y;
		yy = fmod(yy, HexGrid_TileSize.y);

		if(xCol % 2) {
			if(yy < (HexGrid_TileSize.y - rightTriHeight) / 2)       return  xCol    * 4 + (yRow-1); 
			else if(yy <= (HexGrid_TileSize.y + rightTriHeight) / 2) return (xCol+1) * 4 +  yRow;
			else                                                     return  xCol    * 4 +  yRow;
		} else {
			if(yy < (HexGrid_TileSize.y - leftTriHeight) / 2)       return (xCol+1) * 4 + (yRow-1); 
			else if(yy <= (HexGrid_TileSize.y + leftTriHeight) / 2) return  xCol    * 4 +  yRow;
			else                                                    return (xCol+1) * 4 +  yRow;
		}
	}
	
	return -1;
}
struct State {
	i32 PlayerTile;
	struct Array_i32 EnemyTiles;
	struct Array_i32 UsedTiles;
}; 

enum TileMoveIsValid {
	TileMove_NotValid = 0,
	TileMove_Valid = 1,
	TileMove_JumpOverEnemy = 2
};

enum TileDir {
	TileDir_Up = 0,
	TileDir_LeftUp,
	TileDir_RightUp,
	TileDir_Down,
	TileDir_LeftDown,
	TileDir_RightDown,

	TileDir_Total
};

const enum TileDir TileDir_Reverse[TileDir_Total] = {
	TileDir_Down,
	TileDir_RightDown,
	TileDir_LeftDown,
	TileDir_Up,
	TileDir_RightUp,
	TileDir_LeftUp
};

const u32 
	MAX_TILE = 43,
	MAX_TILE_X = 10,
	MAX_TILE_Y = 3;

i32 HexGrid_GetNeighbouringTile(i32 tile, enum TileDir dir) {
	u32 x = tile / (MAX_TILE_Y+1), 
		y = tile % (MAX_TILE_Y+1);

	switch(dir)
	{
		case TileDir_Up:
			if(y == 0) return -1;
			else       return tile - 1;

		case TileDir_Down:
			if(y == MAX_TILE_Y) return -1;
			else                return tile + 1;

		case TileDir_LeftDown:
			if(x == 0 || (x % 2 == 1 && y == MAX_TILE_Y)) 
				return -1;
			else 
				return tile - 4 + (x % 2 ? 1 : 0);

		case TileDir_RightDown:
			if(x == MAX_TILE_X || (x % 2 == 1 && y == MAX_TILE_Y)) 
				return -1;
			else 
				return tile + 4 + (x % 2 ? 1 : 0);

		case TileDir_LeftUp:
			if(x == 0 || (x % 2 == 0 && y == 0)) 
				return -1;
			else 
				return tile - 4 - (x % 2 ? 0 : 1);

		case TileDir_RightUp:
			if(x == MAX_TILE_X || (x % 2 == 0 && y == 0))
				return -1;
			else
				return tile + 4 - (x % 2 ? 0 : 1);

		default:
			Log(WARN, "Unknown value %d for enum TileDir.", dir);
			return -1;
	}
}
enum TileDir HexGrid_GetDir(i32 t1, i32 t2)
{
	for(enum TileDir i = TileDir_Up; i < TileDir_Total; ++i)
		if(HexGrid_GetNeighbouringTile(t1, i) == t2)
			return i;

	return TileDir_Total;
}

bool8 HexGrid_TilesAreNeighbours(i32 t1, i32 t2)
{
	for(enum TileDir i = TileDir_Up; i < TileDir_Total; ++i)
		if(HexGrid_GetNeighbouringTile(t1, i) == t2)
			return 1;

	return 0;
}

enum TileMoveIsValid
HexGrid_TileIsValidMove(const struct State *state, i32 tile)
{
	if(!state || state->PlayerTile < 0) {
		return TileMove_NotValid;
	}

	// FIXME: The upper value for this is hardcoded.
	//        This will probably need fixing if there is to be
	//        more than one level for this game.
	if(tile < 0 || tile > MAX_TILE)
		return TileMove_NotValid;

	if(!HexGrid_TilesAreNeighbours(tile, state->PlayerTile))
		return TileMove_NotValid;

	bool8 tileOccupied = 0;
	for(u32 i = 0; i < state->EnemyTiles.Size; ++i)
	{
		if(tile == state->EnemyTiles.Data[i]) {
			tileOccupied = 1;
			break;
		}
	}

	if(!tileOccupied) {
		for(u32 i = 0; i < state->UsedTiles.Size; ++i){
			if(tile == state->UsedTiles.Data[i])
				return TileMove_NotValid;
		}
		return TileMove_Valid;
	}

	// If the tile is occupied, check if there's a possible move 
	// beyond it in the same direction.
	enum TileDir dir = TileDir_Up;
	for(; dir < TileDir_Total; ++dir)
		if(HexGrid_GetNeighbouringTile(tile, dir) == state->PlayerTile)
			break;

	dir = TileDir_Reverse[dir];

	i32 neighbour = HexGrid_GetNeighbouringTile(tile, dir);
	if(neighbour < 0)
		return TileMove_NotValid;

	tileOccupied = 0;
	for(u32 i = 0; i < state->EnemyTiles.Size; ++i)
	{
		if(neighbour == state->EnemyTiles.Data[i]) {
			tileOccupied = 1;
			break;
		}
	}

	for(u32 i = 0; i < state->UsedTiles.Size; ++i){
		if(neighbour == state->UsedTiles.Data[i])
			return TileMove_NotValid;
	}
	return (tileOccupied ? TileMove_NotValid : TileMove_JumpOverEnemy);
}

struct RSys_Texture tileSelect;
struct RSys_Texture tileUsed;
void HexGrid_DrawTile(i32 tile, bool8 used)
{
	if(tile < 0)
		return;

	u32 w = RSys_GetSize().Width,
		h = RSys_GetSize().Height;

	Vec2 pos = HexGrid_GetTilePos(tile);
	Vec2 size = Vec2_MultVec(HexGrid_TileSize, V2(w, h));
	R2D_DrawRectImage(pos, size, used ? tileUsed.Id : tileSelect.Id, NULL);
}

int main(int argc, char *argv[]) {
	u32 StartupTime = SDL_GetTicks();

	RSys_Init(1280, 720);

	SDL_Event e;
	u32 Ticks = 0;

	Log(Log_Info, "Startup time: %u ms\n", SDL_GetTicks() - StartupTime);
	SDL_GL_SetSwapInterval(-1);

	struct RSys_Texture map = RSys_TextureFromFile("Map.png");
	tileSelect = RSys_TextureFromFile("res/textures/tile_select.png");
	tileUsed   = RSys_TextureFromFile("res/textures/tile_used.png");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float Time = 0;

	u32 MouseX = 0, MouseY = 0;
	enum Mode {
		Mode_Play,
		Mode_Edit
	} Mode = Mode_Play;

	bool8 ShowControls = 0;

	struct State EditState, GameState, *CurrState;
	CurrState = &EditState;

	memset(&EditState, 0, sizeof(struct State));
	memset(&GameState, 0, sizeof(struct State));

	EditState.PlayerTile = -1;

	while(1) {
		Ticks = SDL_GetTicks();

		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: goto end;
				case SDL_KEYUP:
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE: 
							if(Mode == Mode_Edit) {
								Mode = Mode_Play; 
								break;
							} else  {
								goto end;
							}
						case SDLK_e:
							switch(Mode)
							{
								case Mode_Edit:
									Mode = Mode_Play;
									CurrState = &GameState;
									GameState.PlayerTile = EditState.PlayerTile;
									for(u32 i = 0; i < EditState.EnemyTiles.Size; i++)
										Array_i32_Push(&GameState.EnemyTiles, EditState.EnemyTiles.Data + i);
									break;
								case Mode_Play:
									Mode = Mode_Edit;
									CurrState = &EditState;

									Array_i32_Free(&GameState.EnemyTiles);
									Array_i32_Free(&GameState.UsedTiles);
									break;
							}
							break;
						case SDLK_r:
							// TODO: Break this out into seperate function.
							Array_i32_Free(&GameState.EnemyTiles);
							Array_i32_Free(&GameState.UsedTiles);

							GameState.PlayerTile = EditState.PlayerTile;
							for(u32 i = 0; i < EditState.EnemyTiles.Size; i++)
								Array_i32_Push(&GameState.EnemyTiles, EditState.EnemyTiles.Data + i);
							break;

						case SDLK_SLASH:
							if(e.key.keysym.mod | KMOD_SHIFT)
								ShowControls = !ShowControls;
							break;
					}
					break;
				case SDL_WINDOWEVENT:
					switch(e.window.event) {
						case SDL_WINDOWEVENT_CLOSE: goto end;
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						case SDL_WINDOWEVENT_RESIZED: {
							i32 w = e.window.data1, h = e.window.data2;
							glViewport(0, 0, w, h);
							break;
						}
					}
					break;
				case SDL_MOUSEMOTION:
					MouseX = e.motion.x;
					MouseY = e.motion.y;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if(e.button.button == SDL_BUTTON_LEFT) {
						i32 tile = HexGrid_SelectTile(V2(MouseX, MouseY));
						if(Mode == Mode_Edit) {
							bool8 tileTaken = 0;
							for(u32 i = 0; tile >= 0 && i < EditState.EnemyTiles.Size; i++) {
								if(EditState.EnemyTiles.Data[i] == tile) { 
									tileTaken = 1;
									break;
								}
							}
							EditState.PlayerTile = (!tileTaken && tile >= 0 ? tile : EditState.PlayerTile);
						} else {
							enum TileMoveIsValid move = HexGrid_TileIsValidMove(&GameState, tile);
							switch(move) {
								case TileMove_NotValid: break;
								case TileMove_Valid:
									Array_i32_PushVal(&GameState.UsedTiles, GameState.PlayerTile);
									GameState.PlayerTile = tile;
									break;
								case TileMove_JumpOverEnemy:
									Array_i32_PushVal(&GameState.UsedTiles, GameState.PlayerTile);
									for(u32 i = 0; i < GameState.EnemyTiles.Size; ++i)
									{
										if(tile == GameState.EnemyTiles.Data[i]) {
											Array_i32_Remove(&GameState.EnemyTiles, i);
											break;
										}
									}
									GameState.PlayerTile = HexGrid_GetNeighbouringTile(tile, HexGrid_GetDir(GameState.PlayerTile, tile));
									break;
							}
						}
					} else if(e.button.button == SDL_BUTTON_RIGHT && Mode == Mode_Edit) {
						i32 tile = HexGrid_SelectTile(V2(MouseX, MouseY));
						if(tile == -1 || tile == EditState.PlayerTile )
							break;

						bool8 removed = 0;
						for(u32 i = 0; i < EditState.EnemyTiles.Size; i++)
						{
							if(EditState.EnemyTiles.Data[i] == tile)
							{
								Array_i32_Remove(&EditState.EnemyTiles, i);
								removed = 1;
								break;
							}
						}
						if(!removed && EditState.EnemyTiles.Size < 5) 
							Array_i32_Push(&EditState.EnemyTiles, &tile);
						break;
					}
					break;
				default: {
					break;
				}
			}
		}

		// Render frame to back buffer.
		if(Ticks - RSys_GetLastFrameTime() > 16) {
			R2D_DrawRectImage(V2(0,0), V2(RSys_GetSize().Width, RSys_GetSize().Height), map.Id, NULL);
			
			i32 tile = HexGrid_SelectTile(V2(MouseX, MouseY));
			enum TileMoveIsValid move = HexGrid_TileIsValidMove(&GameState, tile);
			switch(move) {
				case TileMove_NotValid:
					HexGrid_DrawTile(tile, 1);
					break;
				case TileMove_Valid:
					HexGrid_DrawTile(tile, 0);
					break;
				case TileMove_JumpOverEnemy:
					HexGrid_DrawTile(tile, 0);
					HexGrid_DrawTile(
							HexGrid_GetNeighbouringTile(tile, HexGrid_GetDir(GameState.PlayerTile, tile)),
							0);
					break;
			}
			u32 w = RSys_GetSize().Width,
				h = RSys_GetSize().Height;

			if(!ShowControls) {
				u32 w = RSys_GetSize().Width;
				Vec2 sz = R2D_GetTextExtents(&R2D_DefaultFont_Large, "? for controls");
				R2D_DrawText(V2(w - sz.x - 10, 10), V4(1,1,1,1), V4(0,0,0,1), &R2D_DefaultFont_Large, "? for controls");
			} else {
				const char* PlayControls =
					" E - Edit mode \n"
					" R - Reset     \n"
					" ESC - Quit    \n"
					" LMB - Move    \n";

				const char* EditControls =
					" E or ESC - Play mode        \n"
					" LMB - Place player          \n"
					" RMB - Place or remove enemy ";

				u32 w = RSys_GetSize().Width;
				Vec2 sz;
				sz = R2D_GetTextExtents(&R2D_DefaultFont_Large, Mode == Mode_Play ? PlayControls : EditControls);
				R2D_DrawText(V2(w - sz.x - 10, 10), V4(1,1,1,1), V4(0,0,0,1), &R2D_DefaultFont_Large, Mode == Mode_Play ? PlayControls : EditControls);
			}

			if(Mode == Mode_Edit) 
			{
				u32 w = RSys_GetSize().Width;
				Vec2 sz = R2D_GetTextExtents(&R2D_DefaultFont_Large, " EDIT MODE ");
				R2D_DrawText(V2((w - sz.x)/2, 10), V4(1,1,1,1), V4(0,0,0,1), &R2D_DefaultFont_Large, " EDIT MODE ");
				R2D_DrawText(
					V2(10, 10), V4(1,1,1,1), V4(0,0,0,1), &R2D_DefaultFont_Large, 
					" Num enemies: %d/%d \n"
					" Player tile: %d \n",
					EditState.EnemyTiles.Size, 5,
					EditState.PlayerTile
				);
			}

			if(Mode == Mode_Play)
			{
				for(u32 i = 0; i < GameState.UsedTiles.Size; ++i)
					HexGrid_DrawTile(GameState.UsedTiles.Data[i], 1);
			}

			if(CurrState->PlayerTile >= 0)
			{
				Vec2 pos = HexGrid_GetTilePos(CurrState->PlayerTile);
				Vec2 size = Vec2_MultVec(HexGrid_TileSize, V2(w, h));
				pos = Vec2_Add(pos, Vec2_MultScal(size, 0.5));

				struct R2D_Rect PlayerRect;

				PlayerRect.Size = V2(70, 110);
				PlayerRect.Position = pos;
				PlayerRect.Position.x -= PlayerRect.Size.x / 2;
				PlayerRect.Position.y -= PlayerRect.Size.y - 5;
				PlayerRect.Color = V4(0.3, 0.3, 0.3, 1);
				R2D_DrawRects(&PlayerRect, 1, 1);

				PlayerRect.Size = V2(60, 100);
				PlayerRect.Position = pos;
				PlayerRect.Position.x -= PlayerRect.Size.x / 2;
				PlayerRect.Position.y -= PlayerRect.Size.y;
				PlayerRect.Color = V4(0.3, 1, 0.3, 1);
				R2D_DrawRects(&PlayerRect, 1, 1);
			}

			for(u32 i = 0; i < CurrState->EnemyTiles.Size; i++)
			{
				Vec2 pos = HexGrid_GetTilePos(CurrState->EnemyTiles.Data[i]);
				Vec2 size = Vec2_MultVec(HexGrid_TileSize, V2(w, h));
				pos = Vec2_Add(pos, Vec2_MultScal(size, 0.5));

				struct R2D_Rect EnemyRect;

				EnemyRect.Size = V2(70, 110);
				EnemyRect.Position = pos;
				EnemyRect.Position.x -= EnemyRect.Size.x / 2;
				EnemyRect.Position.y -= EnemyRect.Size.y - 5;
				EnemyRect.Color = V4(0.3, 0.3, 0.3, 1);
				R2D_DrawRects(&EnemyRect, 1, 1);

				EnemyRect.Size = V2(60, 100);
				EnemyRect.Position = pos;
				EnemyRect.Position.x -= EnemyRect.Size.x / 2;
				EnemyRect.Position.y -= EnemyRect.Size.y;
				EnemyRect.Color = V4(1.0, 0.6, 0.3, 1);
				R2D_DrawRects(&EnemyRect, 1, 1);
			}

			// Display the work onto the screen.
			RSys_FinishFrame();
			Time += 1e-2;
		}

		Ticks = SDL_GetTicks();
	}

end:
	RSys_Quit();
}
