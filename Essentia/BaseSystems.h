#pragma once

#include "Entity.h"
#include "System.h"
#include "Engine.h"
#include "imgui.h"
#include "Serialization.h"

using namespace DirectX;


class EditorSaveSystem : public ISystem
{
public:
	virtual void Update(float dt, float totalTime) override
	{
		currentTime += dt;
		if (keyboard.LeftAlt && currentTime > 0.5f)
		{
			currentTime = 0;
			auto transform = GetTransform({ 2 });
			Save(transform, "test.json");
			std::cout << "Saved\n";
		}
	}

private:
	float currentTime = 0.f;
};

class TransformUpdateSystem : public ISystem
{
public:
	virtual void Update(float dt, float totalTime) override
	{
		uint32 count;
		auto pos = GetComponents<PositionComponent>(count);
		auto rot = GetComponents<RotationComponent>(count);
		auto scale = GetComponents<ScaleComponent>(count);
		auto entities = GetEntities<PositionComponent>(count);
		for (uint32 i = 0; i < count; ++i)
		{
			auto Pos = pos[i];
			auto Rot = rot[i];
			auto Scale = scale[i];
			Transform transform = { Pos, Rot, Scale };
			entityManager->UpdateTransform(entities[i], transform);
		}
	}
};

class RotationSystem : public ISystem
{
public:
	virtual void Initialize()
	{
		entity = entityManager->CreateEntity();
		entity2 = entityManager->CreateEntity();
		auto e = entityManager->CreateEntity();
		lights[0] = entityManager->CreateEntity();
		lights[1] = entityManager->CreateEntity();
		skybox = entityManager->CreateEntity();
		entityManager->AddComponent<SkyboxComponent>(skybox, SkyboxComponent::Create("../../Assets/Textures/SunnyCubeMap.dds"));
		XMFLOAT3 direction;
		auto dir = XMVector3Normalize(XMVectorSet(1, -1, 1, 0));
		XMStoreFloat3(&direction, dir);
		entityManager->AddComponent<DirectionalLightComponent>(lights[0], DirectionalLightComponent::Create(direction, XMFLOAT3(0.9f, 0.9f, 0.9f)));

		entityManager->AddComponent<PointLightComponent>(lights[1], PointLightComponent::Create(XMFLOAT3(0.9f, 0.1f, 0.1f)));
		auto transform = GetTransform(lights[1]);
		transform.Position->y = 3;
		MaterialHandle mat = { 0 };
		MeshHandle mesh = { 1 };
		MeshHandle cone = Es::CreateMesh("../../Assets/Models/cube.obj");

		entityManager->AddComponent<DrawableComponent>(entity, DrawableComponent::Create(mesh, mat));
		entityManager->AddComponent<DrawableComponent>(entity2, DrawableComponent::Create(cone, mat));
		entityManager->AddComponent<DrawableModelComponent>(e, DrawableModelComponent::Create({ 0 }));

		entityManager->AddComponent<SelectedComponent>(entity);
		transform = GetTransform(e);
		transform.Position->z = 4;
		auto scale = XMFLOAT3(0.05f, 0.05f, 0.05f);
		memcpy(transform.Scale, &scale, sizeof(scale));

		transform = GetTransform(entity2);
		transform.Position->z = 5;
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		auto transform = GetTransform(entity);
		transform.Rotation->y = totalTime / 2;
		transform.Position->x = 3 * sin(totalTime * 2);
		transform = GetTransform(entity2);
		//transform.Rotation->y = totalTime;
		//transform.Position->y = cos(totalTime);
	}

private:
	EntityHandle entity;
	EntityHandle entity2;
	EntityHandle lights[2];
	EntityHandle skybox;
};

class FreeCameraSystem : public ISystem
{
public:
	void Initialize()
	{
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		bool debugNav = false;
#ifdef _DEBUG
		ImGuiIO& io = ImGui::GetIO();
		debugNav = io.NavActive;
#endif
		auto up = XMVectorSet(0, 1, 0, 0); // Y is up!
		auto dir = XMLoadFloat3(&camera->Direction);
		auto pos = XMLoadFloat3(&camera->Position);

		float Speed = this->Speed;

		if (keyboard.IsKeyDown(DirectX::Keyboard::LeftShift))
		{
			Speed *= 6.f;
		}

		if (keyboard.IsKeyDown(DirectX::Keyboard::A))
		{
			auto leftDir = XMVector3Cross(dir, up);
			pos = pos + leftDir * deltaTime * Speed;
		}

		if (keyboard.IsKeyDown(DirectX::Keyboard::D))
		{
			auto rightDir = XMVector3Cross(-dir, up);
			pos = pos + rightDir * deltaTime * Speed;
		}

		if (keyboard.IsKeyDown(DirectX::Keyboard::W))
		{
			pos = pos + dir * deltaTime * Speed;
		}

		if (keyboard.IsKeyDown(DirectX::Keyboard::S))
		{
			pos = pos - dir * deltaTime * Speed;
		}

		float xDiff = 0;
		float yDiff = 0;

		if (mouse.leftButton && !debugNav) //Don't move if imgui is active
		{
			xDiff = (float)(mouse.x - prevPos.x) * 0.005f;
			yDiff = (float)(mouse.y - prevPos.y) * 0.005f;
		}

		XMStoreFloat3(&camera->Position, pos);

		camera->Rotation.x += yDiff;
		camera->Rotation.y += xDiff;
		prevPos.x = (float)mouse.x;
		prevPos.y = (float)mouse.y;
	}

	DirectX::XMFLOAT2 prevPos = {};
	float Speed = 20.f;
};