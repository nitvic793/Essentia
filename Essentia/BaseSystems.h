#pragma once

#include "Entity.h"
#include "System.h"
#include "Engine.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Serialization.h"
#include "Renderer.h"

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
		auto materialData = ec->ShaderResourceManager->GetAllMaterialData();
		auto materialNames = ec->ShaderResourceManager->GetAllMaterialNames();
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

		std::vector<MaterialInterface> materials;
		uint32 index = 0;
		for (auto& matName : materialNames)
		{
			MaterialInterface material{ matName };
			for (uint32 i = 0; i < materialData[index].TextureCount; ++i)
			{
				auto name = GContext->ShaderResourceManager->GetTextureName(materialData[index].Textures[i]);
				material.Textures.push_back(name);
			}
			materials.push_back(material);
			index++;
		}

		::SaveScene({ resources, materials, entityList }, "../../Scene/scene.json");
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
		uint32 count = 0;
		auto camEntity = entityManager->GetEntities<CameraComponent>(count);
		auto transform = entityManager->GetTransform(camEntity[0]);
		auto cameras = entityManager->GetComponents<CameraComponent>(count);
		auto camera = cameras[0].CameraInstance;
		bool debugNav = false;
#ifdef EDITOR
		auto stages = GRenderStageManager.GetRenderStageMap();
		ImGuiIO& io = ImGui::GetIO();
		debugNav = (io.NavActive || ImGuizmo::IsUsing() || ImGui::IsAnyItemFocused()) && stages["ImguiRenderStage"]->Enabled;
#else
		debugNav = false;
#endif
		auto up = XMVectorSet(0, 1, 0, 0); // Y is up!
		auto dir = XMLoadFloat3(&camera.Direction);
		auto pos = XMLoadFloat3(transform.Position);

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

		if (!debugNav)
			XMStoreFloat3(transform.Position, pos);

		transform.Rotation->x += yDiff;
		transform.Rotation->y += xDiff;
		prevPos.x = (float)mouse.x;
		prevPos.y = (float)mouse.y;
	}

	DirectX::XMFLOAT2 prevPos = {};
	float Speed = 20.f;
};

class UpdateCameraSystem : public ISystem
{
public:
	void Initialize()
	{}

	virtual void Update(float deltaTime, float totalTime) override
	{
		uint32 count = 0;
		auto entities = GetEntities<CameraComponent>(count);
		auto components = GetComponents<CameraComponent>(count);
		for (uint32 i = 0; i < count; ++i)
		{
			auto position = entityManager->GetComponent<PositionComponent>(entities[i]);
			auto rotation = entityManager->GetComponent<RotationComponent>(entities[i]);
			components[i].CameraInstance.Position = *position;
			components[i].CameraInstance.Rotation = *rotation;
			components[i].CameraInstance.Update(deltaTime, totalTime);
		}

		// Assumption - Camera at index 0 is main camera
		if (count > 0)
		{
			auto size = GContext->RendererInstance->GetScreenSize();
			components[0].CameraInstance.Width = (float)size.Width;
			components[0].CameraInstance.Height = (float)size.Height;
		}
	}
};

class UpdateBaseDrawablesSystem : public ISystem
{
public:
	void Initialize()
	{}

	virtual void Update(float deltaTime, float totalTime) override
	{
		auto shaderResourceManager = GContext->ShaderResourceManager;
		uint32 count = 0;
		auto entities = GetEntities<CameraComponent>(count);
		auto components = GetComponents<CameraComponent>(count);
		const Camera& camera = components[0].CameraInstance;
		auto imageIndex = GContext->RendererInstance->GetCurrentBackbufferIndex();
		auto drawables = GetComponents<BaseDrawableComponent>(count);

		entities = GetEntities<BaseDrawableComponent>(count);
		auto worlds = entityManager->GetTransposedWorldMatrices(entities, count);
		auto transform = entityManager->GetTransform(entities[0]);
		transform.Position->x;
		auto projection = XMLoadFloat4x4(&camera.Projection);
		auto view = XMLoadFloat4x4(&camera.View);

		PerObjectConstantBuffer perObject;
		XMStoreFloat4x4(&perObject.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&perObject.Projection, XMMatrixTranspose(projection));
		for (size_t i = 0; i < count; ++i)
		{
			auto world = XMMatrixTranspose(XMLoadFloat4x4(&worlds[i]));
			XMStoreFloat4x4(&drawables[i].WorldViewProjection, XMMatrixTranspose(world * view * projection));
			drawables[i].World = worlds[i];
			perObject.PrevWorldViewProjection = drawables[i].PrevWorldViewProjection;
			perObject.World = worlds[i];
			perObject.WorldViewProjection = drawables[i].WorldViewProjection;
			shaderResourceManager->CopyToCB(imageIndex, { &perObject, sizeof(perObject) }, drawables[i].CBView.Offset);
			drawables[i].PrevWorldViewProjection = drawables[i].WorldViewProjection;
		}
	}
};