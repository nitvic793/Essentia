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
	virtual void Initialize() override
	{
		//auto scene = LoadLevel("scene.json");
	}

	virtual void Update(float dt, float totalTime) override
	{

		currentTime += dt;
		if (keyboard.LeftAlt && currentTime > 0.5f)
		{
			currentTime = 0;
			auto transform = GetTransform({ 2 });
			//Save(transform, "test.json");
			SaveScene();
			std::cout << "Saved\n";
		}
	}

private:
	bool flip = true;
	void SaveScene()
	{
		ResourcePack resources;
		//resources.Textures.push_back("../../Assets/Textures/floor_albedo.png");
		//resources.Textures.push_back("../../Assets/Textures/floor_normals.png");
		//resources.Textures.push_back("../../Assets/Textures/floor_roughness.png");
		//resources.Textures.push_back("../../Assets/Textures/floor_metal.png");
		//resources.Textures.push_back("../../Assets/Textures/defaultRoughness.png");
		//resources.Textures.push_back("../../Assets/Textures/defaultMetal.png");
		//resources.CubeMaps.push_back("../../Assets/Textures/SunnyCubeMap.dds");
		//resources.CubeMaps.push_back("../../Assets/Textures/envDiffuseHDR.dds");
		//resources.CubeMaps.push_back("../../Assets/Textures/envBrdf.dds");
		//resources.CubeMaps.push_back("../../Assets/Textures/envSpecularHDR.dds");
		auto ec = EngineContext::Context;
		auto meshes = ec->MeshManager->GetAllMeshNames();
		auto models = ec->ModelManager->GetAllModelNames();
		auto textures = ec->ShaderResourceManager->GetAllTextures();
		for (auto mesh : meshes)
		{
			resources.Meshes.push_back(mesh);
		}

		for (auto model : models)
		{
			resources.Models.push_back(model);
		}

		for (auto texture : textures)
		{
			resources.Textures.push_back(texture);
		}

		auto componentManager = entityManager->GetComponentManager();
		flip ? componentManager->RemoveComponent<SelectedComponent>({ 0 }) : componentManager->AddComponent<SelectedComponent>({ 0 });
		flip = !flip;
		uint32 count = 0;
		auto entities = GetEntities<PositionComponent>(count);

		std::vector<EntityInterface> entityList;
		for (uint32 i = 0; i < count; ++i)
		{
			EntityInterface eInterface;
			eInterface.Entity = entities[i];
			auto components = entityManager->GetComponents(entities[i]);
			for (auto comp : components)
			{
				eInterface.Components.push_back(comp.ComponentName.data());
			}
			entityList.push_back(eInterface);
		}

		::SaveScene({ resources, entityList }, "../../Scene/scene.json");
	}

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

class FreeCameraSystem : public ISystem
{
public:
	void Initialize()
	{
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		bool debugNav = false;
#ifdef EDITOR
		auto stages = GRenderStageManager.GetRenderStageMap();
		ImGuiIO& io = ImGui::GetIO();
		debugNav = io.NavActive && stages["ImguiRenderStage"]->Enabled;
#else
		debugNav = false;
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