#include "GameLoader.h"
#include "../Essentia/Game.h"

void GameLoader::InitializeLoader(EngineContext* context)
{
	initializeFunc(context);
}

void GameLoader::LoadSystems(Game* game, IAllocator* allocator)
{
	auto manager = game->GetGameSystemsManager();
	manager->Destroy();
	loadSystemsFunc(manager, allocator);
	manager->Initialize();
}

Game* GameLoader::CreateGame(IAllocator* allocator)
{
	return (Game*)createGameFunc(allocator);
}

void GameLoader::LoadGameLibrary(const char* dll)
{
	hInstLib = LoadLibraryA(dll);
	if (hInstLib)
	{
		loadSystemsFunc = (LoadSystemsFunc)GetProcAddress(hInstLib, "LoadSystems");
		initializeFunc = (InitializeFunc)GetProcAddress(hInstLib, "Initialize");
		createGameFunc = (CreateGameFunc)GetProcAddress(hInstLib, "CreateGame");
	}
}

void GameLoader::FreeGameLibrary()
{
	if (hInstLib != NULL)
	{
		result = FreeLibrary(hInstLib);
	}
}
