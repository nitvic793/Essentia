#include "pch.h"
#include "Interface.h"
#include "System.h"
#include "Systems.h"
#include "Memory.h"
#include "EngineContext.h"
#include "MyGame.h"

void Initialize(EngineContext* context)
{	
	EngineContext::Context = context;
	GContext = context;
	InitializeResources();
}

void* CreateGame(IAllocator* allocator)
{
	return nullptr;
}

void LoadSystems(SystemManager* systemManager, IAllocator* allocator)
{
	systemManager->RegisterSystem<RotationSystem>(allocator);
}
