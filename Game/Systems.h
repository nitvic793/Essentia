#pragma once

#include "System.h"
#include <DirectXMath.h>

using namespace DirectX;
class RotationSystem : public ISystem
{
public:
	virtual void Initialize()
	{
		entity = { 0 };
		entity2 = { 1 };
		auto e = { 2 };
		lights[0] = { 3 };
		lights[1] = { 4 };
		skybox = { 5 };
		cManager = entityManager->GetComponentManager();
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		auto entities = cManager->GetEntities<PositionComponent, DrawableComponent>();

		auto transform = GetTransform(entities[0]);
		transform.Rotation->y = totalTime / 2;
		transform.Position->x = 2 * sin(totalTime * 2);
		transform.Position->y = 1;
		transform = GetTransform(entities[1]);

		float speed = totalTime / 5;
		transform.Rotation->x = speed;
		transform.Rotation->y = speed;
		transform.Position->y = 5 + cos(totalTime);

		entities = cManager->GetEntities<PointLightComponent>();
		for (auto e : entities)
		{
			float factor = abs(sin(totalTime));
			auto component = cManager->GetComponent<PointLightComponent>(e);
			component->Intensity = factor * 30.f;
			component->Range = factor * 30.f;
		}
	}

private:
	ComponentManager* cManager;
	EntityHandle entity;
	EntityHandle entity2;
	EntityHandle lights[2];
	EntityHandle skybox;
};