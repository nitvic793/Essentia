#include "pch.h"
#include "Interface.h"
#include "System.h"
#include "Systems.h"
#include "Memory.h"
#include "EngineContext.h"
#include "MyGame.h"
#include "ComponentReflector.h"



void Initialize(EngineContext* context)
{
	EngineContext::Context = context;
	GContext = context;
	InitializeResources();
	GComponentReflector = *context->ComponentReflector;
}

void* CreateGame(IAllocator* allocator)
{
	return nullptr;
}

void LoadSystems(SystemManager* systemManager, IAllocator* allocator)
{
	systemManager->RegisterSystem<RotationSystem>(allocator);

	GContext->ComponentReflector->RegisterComponent<TestComponent>(
		{
			Field{kFieldTypeFloat, 0, "TestValue", "float"},
			Field{kFieldTypeInt32, sizeof(float), "TestInt", "int32"}
		});

}
