#pragma once

#include "System.h"
#include "RenderComponents.h"
#include <DirectXMath.h>

using namespace DirectX;

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


class RotationSystem : public ISystem
{
public:
	virtual void Initialize()
	{
		cManager = entityManager->GetComponentManager();
		//auto e = entityManager->CreateEntity(DefaultTransform, 1);
		//entityManager->AddComponent<DrawableComponent>(e, DrawableComponent::Create({ 0 }, { 0 }));
		//entityManager->AddComponent<TestComponent>(e);
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		auto entities = cManager->GetEntities<PositionComponent, DrawableComponent>();

		auto transform = GetTransform(entities[0]);
		transform.Rotation->y = totalTime / 2;
		transform.Position->x = 9.1f * sin(totalTime * 2);
		transform.Position->y = 1;

		transform = GetTransform(entities[1]);
		float speed = totalTime / 5;
		transform.Rotation->x = speed;
		transform.Rotation->y = speed;
		transform.Position->y = 5 + cos(totalTime);

		entities = cManager->GetEntities<PointLightComponent>();
		for (auto e : entities)
		{
			transform = GetTransform(e);
			transform.Position->z = 10 * sin(totalTime);
			float factor = abs(cos(totalTime));
			auto component = cManager->GetComponent<PointLightComponent>(e);
			//component->Intensity = 1 + factor * 2.f;
			//component->Range = 2 + factor * 8.f;
		}
	}

private:
	ComponentManager* cManager;
};