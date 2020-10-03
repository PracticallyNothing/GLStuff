#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Common.h"
#include "Math3D.h"
#include "Render.h"
#include "SDL_video.h"
#include "Shader.h"
#include "JSON.h"

const struct JSON_Value*
JSON_ObjectFind(const struct JSON_Value *v, const char* key)
{
	if(v->Type != JSON_Object)
		return NULL;

	u128 hash = Hash_MD5((u8*) key, strlen(key));
	i32 i = Array_Hash_Find(&v->Object.Keys, &hash);

	return (i >= 0 ? v->Object.Values.Data + i : NULL);
}

RGB JSON_ArrayToRGB(const struct JSON_Value *arr)
{
	return V3(
		arr->Array.Data[0].Number / 255,
		arr->Array.Data[1].Number / 255,
		arr->Array.Data[2].Number / 255
	);
}

u32 PrevDay(u32 Weekday) { return Weekday == 0 ? 6 : Weekday-1; }
u32 NextDay(u32 Weekday) { return Weekday == 6 ? 0 : Weekday+1; }

r32 ParseTime(const char* str)
{
	const char* s = strchr(str, ':');
	return (String_ToR32_N(str, s - str) + (String_ToR32_N(s+1, strlen(s+1))/60.0))/ 24.0;
}

int main(int argc, char *argv[]) {
	u32 StartupTime = SDL_GetTicks();
	srand(time(NULL));

	RSys_Init(640, 480);

	SDL_Event e;
	u32 Ticks = 0;

	SDL_GL_SetSwapInterval(-1);

	float Time = 0;

	RGB text, bg, notFree, free, unknown;

	struct JSON_Value v = JSON_FromFile("Schedules.json");
	const struct JSON_Value 
		*settings = JSON_ObjectFind(&v, "settings"),
		*colors   = JSON_ObjectFind(settings, "colors"),
		*jsonText    = JSON_ObjectFind(colors, "text"),
		*jsonBg      = JSON_ObjectFind(colors, "bg"),
		*jsonNotFree = JSON_ObjectFind(colors, "not-free"),
		*jsonFree    = JSON_ObjectFind(colors, "free"),
		*jsonUnknown = JSON_ObjectFind(colors, "unknown");


	if(jsonText)    text    = JSON_ArrayToRGB(jsonText);
	if(jsonBg)      bg      = JSON_ArrayToRGB(jsonBg);
	if(jsonNotFree) notFree = JSON_ArrayToRGB(jsonNotFree);
	if(jsonFree)    free    = JSON_ArrayToRGB(jsonFree);
	if(jsonUnknown) unknown = JSON_ArrayToRGB(jsonUnknown);

	glClearColor(bg.r, bg.g, bg.b, 1);

	const struct JSON_Value *schedules = JSON_ObjectFind(&v, "schedules");
	r32 MaxNameLength = 0;

	for(u32 i = 0; i < schedules->Array.Size; ++i)
	{
		const struct JSON_Value* sc = JSON_ObjectFind(schedules->Array.Data + i, "name-en");
		MaxNameLength = MAX(MaxNameLength, R2D_GetTextExtents(&R2D_DefaultFont_Large, sc->String).x);
	}

	const char* WeekdayStrings[] = {
		"MONDAY",
		"TUESDAY",
		"WEDNESDAY",
		"THURSDAY",
		"FRIDAY",
		"SATURDAY",
		"SUNDAY"
	};
	u32 Weekday = 0;

	Vec2 MousePos;

	bool8 Draw = 1;
	Log(Log_Info, "Startup time: %u ms", SDL_GetTicks() - StartupTime);

	while(1) {
		Ticks = SDL_GetTicks();

		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: goto end;
				case SDL_KEYUP: {
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE: goto end;
						case SDLK_1: Weekday = 0; break;
						case SDLK_2: Weekday = 1; break;
						case SDLK_3: Weekday = 2; break;
						case SDLK_4: Weekday = 3; break;
						case SDLK_5: Weekday = 4; break;
						case SDLK_6: Weekday = 5; break;
						case SDLK_7: Weekday = 6; break;
						case SDLK_LEFT:  Weekday = PrevDay(Weekday); break;
						case SDLK_RIGHT: Weekday = NextDay(Weekday); break;
						case SDLK_TAB:   Weekday = (e.key.keysym.mod & KMOD_SHIFT ? PrevDay(Weekday) : NextDay(Weekday)); break;
						default: break;
					}
				} break;
				case SDL_WINDOWEVENT: {
					switch(e.window.event) {
						case SDL_WINDOWEVENT_CLOSE: goto end;
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						case SDL_WINDOWEVENT_RESIZED: {
							i32 w = e.window.data1, h = e.window.data2;
							glViewport(0, 0, w, h);
							break;
						}
						case SDL_WINDOWEVENT_FOCUS_GAINED:
						case SDL_WINDOWEVENT_EXPOSED:
						case SDL_WINDOWEVENT_ENTER:
							Draw = 1;
							break;
						case SDL_WINDOWEVENT_LEAVE:
						case SDL_WINDOWEVENT_FOCUS_LOST:
						case SDL_WINDOWEVENT_HIDDEN:
							Draw = 0;
							break;
						default:
							Log(INFO, "Unhandled window event %d.", e.window.event);
							break;
					}
				} break;
				case SDL_MOUSEMOTION: {
					MousePos = V2(e.motion.x, e.motion.y);
				} break;
				case SDL_MOUSEBUTTONDOWN: {} break;
				case SDL_MOUSEWHEEL:      {} break;
				case SDL_MOUSEBUTTONUP:   {} break;
				default: {} break;
			}
		}

		// Render frame to back buffer.
		if(Ticks - RSys_GetLastFrameTime() > 16 && Draw) {
			const r32 padding = 20;

			RSys_Size sz = RSys_GetSize();
			r32 textHeight = 0;

			{
				Vec2 szt[7] = {0};
				r32 sumSz = padding*6;

				for(u32 i = 0; i < 7; ++i) {
					Vec2 textSz = R2D_GetTextExtents(&R2D_DefaultFont_Large, WeekdayStrings[i]);
					sumSz += textSz.x;
					szt[i] = textSz;
				}

				r32 pos = sz.Width/2 - sumSz/2;

				for(u32 i = 0; i < 7; ++i)
				{
					R2D_DrawText(
						V2(pos, 10),
						V4_V3(text, (Weekday == i ? 1 : 0.5)),
						V4(0,0,0,0),
						&R2D_DefaultFont_Large,
						WeekdayStrings[i]
					);
					pos += szt[i].x + 20;
				}
				textHeight = szt[0].y;
			}

			// Draw separator
			struct R2D_Rect r;
			r.Position = V2(0, textHeight + 10 + 5);
			r.Size = V2(sz.Width, 2);
			r.Color = V4_V3(text, 1);

			R2D_DrawRects(&r, 1, 1);
			
			// Draw names
			r32 penY = textHeight + 10 + 5 + 50;

			r.Position = V2(40 + MaxNameLength, textHeight+10+5);
			r.Size = V2(2, sz.Height - (textHeight+10+5));
			R2D_DrawRects(&r, 1, 1);

			r.Color = V4_V3(text, 0.7);
			r.Position = V2(0, penY);
			r.Size = V2(sz.Width, 1);
			R2D_DrawRects(&r, 1, 1);

			for(u32 i = 0; i < schedules->Array.Size; ++i)
			{
				const struct JSON_Value 
					*sc = schedules->Array.Data + i,
					*name = JSON_ObjectFind(sc, "name-en"),
					*schedule = JSON_ObjectFind(sc, "schedule");

				Vec2 textSz = 
					R2D_GetTextExtents(&R2D_DefaultFont_Large, name->String);

				bool8 b = schedule->Array.Data[Weekday].Boolean;
				if(schedule->Array.Data[Weekday].Type == JSON_Boolean) 
				{
					struct R2D_Rect r2 = {
						.Position = V2(40 + MaxNameLength + 2, penY+1),
						.Size     = V2(sz.Width - MaxNameLength - 2, 20 + textSz.y - 2),
						.Color    = V4_V3((b ? free : notFree), 1),
					};
					R2D_DrawRects(&r2, 1, 1);
				}
				else if(schedule->Array.Data[Weekday].Type == JSON_Null) 
				{
					struct R2D_Rect r2 = {
						.Position = V2(40 + MaxNameLength + 2, penY+1),
						.Size     = V2(sz.Width - MaxNameLength - 2, 20 + textSz.y - 2),
						.Color    = V4_V3(unknown, 1),
					};
					R2D_DrawRects(&r2, 1, 1);
				} 
				else if(schedule->Array.Data[Weekday].Type == JSON_Array) 
				{
					const struct JSON_Value *dayParts = schedule->Array.Data + Weekday;

					struct R2D_Rect *rects = 
						Allocate(sizeof(struct R2D_Rect) * dayParts->Array.Size);

					for(u32 i = 0; i < dayParts->Array.Size; ++i)
					{
						const struct JSON_Value 
							*start = dayParts->Array.Data[i].Array.Data,
							*end   = dayParts->Array.Data[i].Array.Data + 1;

						r32 startF = ParseTime(start->String),
							endF   = ParseTime(end->String);

						r32 TotalSchedWidth = sz.Width - MaxNameLength - 2;

						rects[i].Position = 
							V2(40 + MaxNameLength + 2 + TotalSchedWidth * startF, penY+1);

						rects[i].Size = 
							V2(TotalSchedWidth * (endF - startF), 20 + textSz.y - 2);

						rects[i].Color = V4_V3(notFree, 1);
					}

					R2D_DrawRects(rects, dayParts->Array.Size, 1);
				}

				penY += 10;
				R2D_DrawText(
					V2(20, penY),
					V4_V3(text, 1),
					V4(0,0,0,0),
					&R2D_DefaultFont_Large,
					name->String
				);

				penY += textSz.y + 10;

				r.Position = V2(0, penY);
				R2D_DrawRects(&r, 1, 1);
			}

			if(MousePos.x > MaxNameLength + 40 && MousePos.y > textHeight + 10 + 5)
			{
				struct R2D_Rect crosshair = {
					.Position = V2(MousePos.x, textHeight + 10+5+1),
					.Size     = V2(1, sz.Height - (textHeight + 10+5)),
					.Color    = V4_V3(text, 1)
				};

				R2D_DrawRects(&crosshair, 1, 1);

				r32 minutes = (MousePos.x - (MaxNameLength+40)) / (sz.Width - (MaxNameLength+40)) * 60 * 24;
				r32 hours   = floor(minutes / 60.0);
				minutes     = floor(fmod(minutes, 60.0));

				//Log(INFO, "Hours: %.5f | Minutes: %.5f", hours, minutes);

				const char* fmt = "%02.0f:%02.0f (%g/%d)";

				Vec2 ext = R2D_GetTextExtents(&R2D_DefaultFont_Large, fmt, hours, minutes, MousePos.x, sz.Width);

				R2D_DrawText(
					V2(
						(MousePos.x - ext.x - 10 < MaxNameLength + 50 ? MousePos.x + 10 : MousePos.x - ext.x - 10),
						textHeight/2 + 10+5+25
					),
					V4_V3(text, 1), 
					V4(0,0,0,0),
					&R2D_DefaultFont_Large, fmt, 
					hours, minutes, MousePos.x, sz.Width
				);

			}

			// Display the work onto the screen.
			RSys_FinishFrame();
			Time += 1e-2;
		}

		Ticks = SDL_GetTicks();
	}

end:
	RSys_Quit();
	Alloc_FreeAll();
	return 0;
}
