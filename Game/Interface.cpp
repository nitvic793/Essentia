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

struct TestComponent : public IComponent
{
	GComponent(TestComponent)

		float	TestValue = 1.f;
	int32	TestInt = 10;
	template<class Archive>
	void save(Archive& archive) const
	{
	};

	template<class Archive>
	void load(Archive& archive)
	{
	};
};

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

	GContext->ComponentReflector->RegisterComponent<TestComponent>(
		{
			Field{FieldTypes::kFieldTypeFloat, 0, "TestValue", "float"},
			Field{FieldTypes::kFieldTypeInt32, sizeof(float), "TestInt", "int32"}
		});

	GContext->ComponentReflector->RegisterComponent<MoveableUnitComponent>(
		{
			Field{ FieldTypes::kFieldTypeFloat3, 0, "TargetPos", "XMFLOAT3" },
			Field{ FieldTypes::kFieldTypeFloat, sizeof(DirectX::XMFLOAT3), "MoveSpeed", "float" },
		});
}
