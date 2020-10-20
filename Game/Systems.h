#pragma once

#include "System.h"
#include "RenderComponents.h"
#include <DirectXMath.h>
#include "PhysicsHelper.h"

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
		uint32 count;
		auto camera = cManager->GetAllComponents<CameraComponent>(count);
		auto entities = cManager->GetEntities<BoundingOrientedBoxComponent, DrawableComponent>();
		float distance;
		for (auto entity : entities)
		{
			BoundingOrientedBoxComponent* component = cManager->GetComponent<BoundingOrientedBoxComponent>(entity);
			if (es::IsIntersecting(component->BoundingOrientedBox, &camera[0].CameraInstance, mouse.x, mouse.y, distance) && mouse.leftButton)
			{
				UnselectEntities();
				cManager->AddComponent<SelectedComponent>(entity);
				break;
			}
		}

		auto transform = GetTransform(entities[0]);
		transform.Rotation->y = totalTime / 2;
		transform.Position->x = 1.2f * sin(totalTime * 2);
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

	void UnselectEntities()
	{
		uint32 selectCount;
		auto selectedEntities = GContext->EntityManager->GetEntities<SelectedComponent>(selectCount);
		for (uint32 i = 0; i < selectCount; ++i)
		{
			GContext->EntityManager->GetComponentManager()->RemoveComponent<SelectedComponent>(selectedEntities[i]);
		}
	}
};