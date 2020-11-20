#pragma once

#include "BaseComponents.h"
#include "RenderComponents.h"
#include "Entity.h"
#include <iostream>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <fstream>
#include <unordered_map>
#include "Math.h"
#include "PostProcessComponents.h"
#include "AnimationComponent.h"
#include "PipelineStates.h"
#include "TerrainComponent.h"

class IVisitor
{
public:
	virtual void Visit(const char* compName, const char* name, float& val) { };
	virtual void Visit(const char* compName, const char* name, DirectX::XMFLOAT3& val) { };
	virtual void Visit(const char* compName, const char* name, MeshHandle& val) { };
	virtual void Visit(const char* compName, const char* name, MaterialHandle& val) { };
	virtual void Visit(const char* compName, const char* name, bool& val) { };
	virtual void Visit(const char* compName, const char* name, uint32& val) { };
	virtual void Visit(const char* compName, const char* name, int32& val) { };
	virtual void Visit(const char* compName, const char* name, std::string_view& val) { };

	template<typename T>
	void Visit(const char* compName, const char* name, T& val) {};
private:
};

struct MeshInterface
{
	std::string_view Name;
};

struct EntityInterface
{
	std::string					Name;
	EntityHandle				Entity;
	EntityHandle				Parent;
	std::vector<std::string>	Components;

	template<typename Archive>
	void save(Archive& archive) const
	{
		auto ec = EngineContext::Context;
		auto componentManager = ec->EntityManager->GetComponentManager();
		auto name = ec->EntityManager->GetEntityName(Entity);
		auto parent = ec->EntityManager->GetParent(Entity);
		archive(
			cereal::make_nvp("Name", std::string(name)),
			CEREAL_NVP(Entity),
			cereal::make_nvp("Parent", parent),
			CEREAL_NVP(Components)
		);

		for (auto component : Components)
		{
			auto pool = componentManager->GetPool(component.c_str());
			pool->Serialize(archive, Entity);
		}
	}

	template<typename Archive>
	void load(Archive& archive)
	{
		auto ec = EngineContext::Context;
		auto componentManager = ec->EntityManager->GetComponentManager();
		archive(
			CEREAL_NVP(Name),
			CEREAL_NVP(Entity),
			CEREAL_NVP(Parent),
			CEREAL_NVP(Components)
		);

		Entity = ec->EntityManager->CreateEntity(DefaultTransform, Parent.Handle.Index, Name);
		for (auto component : Components)
		{
			auto pool = componentManager->GetPool(component.c_str());
			pool->Deserialize(archive, Entity);
		}

	}
};

struct MaterialInterface
{
	std::string					MaterialName;
	std::vector<std::string>	Textures;

	template<typename Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(MaterialName),
			CEREAL_NVP(Textures)
		);
	}

	template<typename Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(MaterialName),
			CEREAL_NVP(Textures)
		);

		std::vector<TextureID> textureIds;
		for (auto& texture : Textures)
		{
			auto extension = texture.substr(texture.size() - 3, 3);
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char c) { return std::tolower(c); });
			TextureType type = (extension == "dds" ? TextureType::DDS : TextureType::WIC);
			TextureID textureId = GContext->ShaderResourceManager->CreateTexture(texture, type);
			textureIds.push_back(textureId);
		}

		Material mat;
		GContext->ShaderResourceManager->CreateMaterial(textureIds.data(), (uint32)textureIds.size(), GPipelineStates.DefaultPSO, mat, MaterialName.c_str());
	}
};

struct ResourcePack
{
	std::vector<TextureProperties> Textures;
	std::vector<std::string> CubeMaps;
	std::vector<std::string> Meshes;
	std::vector<std::string> Models;

	template<class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(Textures),
			CEREAL_NVP(CubeMaps),
			CEREAL_NVP(Meshes),
			CEREAL_NVP(Models)
		);
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(Textures),
			CEREAL_NVP(CubeMaps),
			CEREAL_NVP(Meshes),
			CEREAL_NVP(Models)
		);

		for (auto texture : Textures)
		{
			es::CreateTexture(texture.Name, texture.TextureLoadType, texture.HasMips);
		}

		for (auto mesh : Meshes)
		{
			es::CreateMesh(mesh);
		}

		for (auto model : Models)
		{
			es::CreateModel(model.c_str());
		}
	}
};

struct Scene
{
	ResourcePack					Resources;
	std::vector<MaterialInterface>	Materials;
	std::vector<EntityInterface>	Entities;

	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(Resources),
			CEREAL_NVP(Materials),
			CEREAL_NVP(Entities));
	}
};


#define MField(component, name) #name, component->name
#define MFieldS(component, name) #name, component.name



void Visit(PositionComponent* component, IVisitor* visitor);
void Visit(RotationComponent* component, IVisitor* visitor);
void Visit(ScaleComponent* component, IVisitor* visitor);
void Visit(PointLightComponent* component, IVisitor* visitor);
void Visit(SpotLightComponent* component, IVisitor* visitor);
void Visit(DirectionalLightComponent* component, IVisitor* visitor);
void Visit(DrawableComponent* component, IVisitor* visitor);
void Visit(DrawableModelComponent* component, IVisitor* visitor);
void Visit(SkyboxComponent* component, IVisitor* visitor);
void Visit(SelectedComponent* component, IVisitor* visitor);
void Visit(PostProcessVolumeComponent* component, IVisitor* visitor);
void Visit(BaseDrawableComponent* component, IVisitor* visitor);
void Visit(CameraComponent* component, IVisitor* visitor);
void Visit(AnimationComponent* component, IVisitor* visitor);
void Visit(BoundingOrientedBoxComponent* component, IVisitor* visitor);
void Visit(TerrainComponent* component, IVisitor* visitor);
