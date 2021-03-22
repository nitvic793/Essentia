#include "pch.h"
#include "Interface.h"
#include "System.h"
#include "Systems.h"
#include "Memory.h"
#include "EngineContext.h"
#include "MyGame.h"
#include "ComponentReflector.h"
#include "MoveableUnitComponent.h"
#include "ObjectInferenceSystem.h"


void Initialize(EngineContext* context)
{
	EngineContext::Context = context;
	GContext = context;
	es::GEventBus = context->EventBus;
	GConsole = context->Console;
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
	systemManager->RegisterSystem<MoveObjectSystem>(allocator);
	systemManager->RegisterSystem<ObjectInferenceSystem>(allocator);

	GContext->ComponentReflector->RegisterComponent<Rotatable>(
		{
			Field{FieldTypes::kFieldTypeFloat, 0, "Speed", "float"},
			Field{FieldTypes::kFieldTypeFloat, sizeof(float), "Rotation", "float"}
		});

	GContext->ComponentReflector->RegisterComponent<MoveableUnitComponent>(
		{
			Field{ FieldTypes::kFieldTypeFloat3, 0, "TargetPos", "XMFLOAT3" },
			Field{ FieldTypes::kFieldTypeFloat, sizeof(DirectX::XMFLOAT3), "MoveSpeed", "float" },
		});
}
