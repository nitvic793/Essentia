
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#define NOMINMAX

#include "../Essentia/Game.h"
#include "GameLoader.h"
#include <dxgidebug.h>

int main()
{
#if defined(DEBUG) | defined(_DEBUG)
	// Enable memory leak detection - quick and dirty
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Ensure "Current Directory" (relative path) is always the .exe's folder
	{
		char currentDir[1024] = {};
		GetModuleFileName(0, currentDir, 1024);
		char* lastSlash = strrchr(currentDir, '\\');
		if (lastSlash)
		{
			*lastSlash = 0;
			SetCurrentDirectory(currentDir);
		}
	}

	{
		GameLoader gameLoader;
		gameLoader.LoadGameLibrary();
		LinearAllocator allocator(CMaxStackHeapSize);
		{
			ScopedPtr<Game> game = MakeScoped<Game>();
			game->Setup();
			gameLoader.InitializeLoader(EngineContext::Context);
			gameLoader.LoadSystems(game.get(), &allocator);
			game->SetSystemReloadCallback([&]() 
			{
				gameLoader.FreeGameLibrary();
				system("msbuild.exe ../../Essentia.sln /target:Game /p:Platform=x64");
				gameLoader.LoadGameLibrary();
				gameLoader.LoadSystems(game.get(), &allocator);
			});
			game->Run();
		}
		gameLoader.FreeGameLibrary();
	}

#ifdef _DEBUG
	IDXGIDebug1* pDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		pDebug->Release();
	}
#endif

	return 0;
}