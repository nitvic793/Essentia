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
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		auto transform = GetTransform(entity);  
		transform.Rotation->y = totalTime / 2;
		transform.Position->x = 2 * sin(totalTime * 2);
		transform.Position->y = 1;
		transform = GetTransform(entity2); 

		float speed = totalTime / 5;
		transform.Rotation->x = speed;
		transform.Rotation->y = speed;
		transform.Position->y = 5 + cos(totalTime);
	}

private:
	EntityHandle entity;
	EntityHandle entity2;
	EntityHandle lights[2];
	EntityHandle skybox;
};