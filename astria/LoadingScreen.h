#ifndef _APP_STATE_LOADING_H_
#define _APP_STATE_LOADING_H_

#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>

#include "Common.h"
#include "Texture_SDL.h"
#include "Params.h"

enum SPARK_TYPE
{
	BLUE_SPARK,
	ORANGE_SPARK,
	PINK_SPARK
};
class CSpark;

class CLoadingScreen
{
public:	
	//Activate the loading screen
	//Loading screen will run on a seperate thread until condition variable becomes true
	static void OnActivate(int* condition);

	static SDL_Thread* GetThreadID();

private:
	static int OnExecute(void* ptr);
	static void OnLoad();
	static void OnUpdate();
	static void OnRender();
	static void OnUnload();
	static void OnExit();

	static int StartTime;
	
	static SDL_Thread* Thread_Loading;
	static int* Condition;
};

class CSpark
{
public:
	CSpark(SPARK_TYPE type, int size, int x, int y);
	static void Load(int size);
	static void AddRandomSpark();
	static void AddSpark(SPARK_TYPE type, int size, int x, int y);
	static int GetCount() { return count; }
	static void Clear();
	static void KillSpark(int ID);

	static void OnRender();

private:
	static std::vector<CSpark> sparkList;
	static std::vector<Texture_SDL*> textureList;
	static int freeID;
	static int count;
	int frame;
	int size;
	int xpos, ypos;
	int ID;
	int maxFrames;
	int framePerRow;
	std::string spriteFile;
	int tileSize;
	SPARK_TYPE type;
};

#endif