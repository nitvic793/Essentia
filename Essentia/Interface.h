#pragma once

#include "BaseComponents.h"
#include "Entity.h"
#include <iostream>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/archives/json.hpp>
#include <fstream>
#include <unordered_map>
#include "Math.h"

class IVisitor
{
public:
	virtual void Visit(const char* compName, const char* name, float& val) { };
	virtual void Visit(const char* compName, const char* name, DirectX::XMFLOAT3& val) { };
private:
};

struct MeshInterface
{
	std::string_view Name;
};

struct EntityInterface
{
	EntityHandle				Entity;
	std::vector<std::string>	Components;

	template<typename Archive>
	void save(Archive& archive) const
	{
		auto ec = EngineContext::Context;
		auto componentManager = ec->EntityManager->GetComponentManager();
		archive(CEREAL_NVP(Entity), CEREAL_NVP(Components));
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
		archive(CEREAL_NVP(Entity), CEREAL_NVP(Components));
		for (auto component : Components)
		{
			auto pool = componentManager->GetPool(component.c_str());
			pool->Deserialize(archive, Entity);
		}

	}
};

struct MaterialInterface
{
	std::vector<std::string> Textures;
};

struct ResourcePack
{
	std::vector<std::string> Textures;
	std::vector<std::string> CubeMaps;
	std::vector<std::string> Meshes;
	std::vector<std::string> Models;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(Textures),
			CEREAL_NVP(CubeMaps),
			CEREAL_NVP(Meshes),
			CEREAL_NVP(Models)
		);
	}
};

struct Scene
{
	ResourcePack					Resources;
	std::vector<EntityInterface>	Entities;

	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(Resources), CEREAL_NVP(Entities));
	}
};


#define MField(component, name) #name, component->name
#define MFieldS(component, name) #name, component.name



void Visit(PositionComponent* component, IVisitor* visitor);
void Visit(RotationComponent* component, IVisitor* visitor);
void Visit(ScaleComponent* component, IVisitor* visitor);
void Visit(PointLightComponent* component, IVisitor* visitor);
void Visit(DirectionalLightComponent* component, IVisitor* visitor);
void Visit(DrawableComponent* component, IVisitor* visitor);
void Visit(DrawableModelComponent* component, IVisitor* visitor);
void Visit(SkyboxComponent* component, IVisitor* visitor);





