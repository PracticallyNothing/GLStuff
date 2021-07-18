#include "Res.h"
#include <SDL.h>

typedef struct LoadJob LoadJob;

enum LoadType{
	Load_Audio,
	Load_Mesh,
	Load_Scene
} Type;

struct LoadJob {
	enum LoadType Type;
	char *File;
};

DEF_ARRAY(LoadJob, LoadJob);
DECL_ARRAY(LoadJob, LoadJob);

static Array_LoadJob JobQueue = {0};

static bool8 Res_Running = 0;
static SDL_mutex *QueueMutex = NULL;
static SDL_Thread *LoadThread = NULL;
static bool8 Res_SingleThreadedMode = 0;

static int Res_Thread(void *data);
static void Res_AddJob(enum LoadType type, const char* file);

static int Res_Thread(void* data)
{
	u32 i = 0;

	while(Res_Running)
	{
		if(i < JobQueue.Size)
		{
			switch(JobQueue.Data[i].Type){
				case Load_Audio: {
				} break;

				case Load_Mesh: {
				} break;

				case Load_Scene: {
				} break;
			}
		}

		if(Res_SingleThreadedMode)
			break;
	}

	return 0;
}

void Res_Init()
{
	if(Res_Running)
		return;

	Res_Running = 1;
	if(!QueueMutex) QueueMutex = SDL_CreateMutex();
	if(!LoadThread) LoadThread = SDL_CreateThread(Res_Thread, "Resource loader", NULL);

	if(!QueueMutex) {
		Log(ERROR, "[Res] Failed to create mutex. %s", SDL_GetError());
		Log(WARN, "[Res] Resource loading will be synchronous.", "");
		Res_SingleThreadedMode = 1;
	} else if(!LoadThread) {
		Log(ERROR, "[Res] Failed to create thread. %s", SDL_GetError());
		Log(WARN, "[Res] Resource loading will be synchronous.", "");
		Res_SingleThreadedMode = 1;
	}
}

void Res_Quit()
{
	SDL_LockMutex(QueueMutex);
	Res_Running = 0;
	SDL_UnlockMutex(QueueMutex);
}

static void Res_AddJob(enum LoadType type, const char* file)
{
	if(!Res_SingleThreadedMode) {
		SDL_LockMutex(QueueMutex);

		LoadJob lj = { 
			.Type = type, 
			.File = malloc(strlen(file)+1)
		};
		strcpy(lj.File, file);
		lj.File[strlen(file)] = '\0';
		Array_LoadJob_Push(&JobQueue, &lj);

		SDL_UnlockMutex(QueueMutex);
	} else {
		Res_Thread(NULL);
	}
}

void Res_LoadAudio(const char* file) { Res_AddJob(Load_Audio, file); }
void Res_LoadMesh (const char* file) { Res_AddJob(Load_Mesh,  file); }
void Res_LoadScene(const char* file) { Res_AddJob(Load_Scene, file); }
