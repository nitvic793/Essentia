#include "Interface.h"
#include "Serialization.h"
#include <functional>
#include <cereal/cereal.hpp>

ComponentReflector GComponentReflector;

void RegisterComponents()
{
	GComponentReflector.RegisterComponent<PositionComponent>();
	GComponentReflector.RegisterComponent<ScaleComponent>();
	GComponentReflector.RegisterComponent<RotationComponent>();
	GComponentReflector.RegisterComponent<PointLightComponent>();
	GComponentReflector.RegisterComponent<DirectionalLightComponent>();
}

void Save(const TransformRef& transform, const char* fname)
{
	{
		TransformInterface transformInt;
		transformInt.Position = *transform.Position;
		transformInt.Scale = *transform.Scale;
		transformInt.Rotation = *transform.Rotation;
		std::ofstream file(fname);
		cereal::JSONOutputArchive archive(file);
		archive(transformInt);
	}
}

void SaveResources(ResourcePack& resources, const char* fname)
{
	std::ofstream file(fname);
	cereal::JSONOutputArchive archive(file);
	archive(cereal::make_nvp("Resources", resources));
}

void SaveEntities(std::vector<EntityInterface>& entities, const char* fname)
{
	std::ofstream file(fname);
	cereal::JSONOutputArchive archive(file);
	archive(cereal::make_nvp("Entities", entities));
}

template<class Archive>
void serialize(Archive& archive, EntityHandle& entity)
{
	archive(CEREAL_NVP(entity.ID));
}

template<class Archive>
void serialize(Archive& archive, Vector3& vector)
{
	archive(vector.X, vector.Y, vector.Z);
}

template<class Archive>
void serialize(Archive& archive, DirectX::XMFLOAT3& vector)
{
	archive(vector.x, vector.y, vector.z);
}

template<class Archive>
void serialize(Archive& archive, TransformInterface& transform)
{
	archive(transform.Position, transform.Rotation, transform.Scale);
}


void Visit(PositionComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->GetName();
	visitor->Visit(name, MField(comp, X));
	visitor->Visit(name, MField(comp, Y));
	visitor->Visit(name, MField(comp, Z));
}

void Visit(RotationComponent* component, IVisitor* visitor)
{
	auto comp = component;
	visitor->Visit(comp->GetName(), MField(comp, X));
	visitor->Visit(comp->GetName(), MField(comp, Y));
	visitor->Visit(comp->GetName(), MField(comp, Z));
}

void Visit(ScaleComponent* component, IVisitor* visitor)
{
	auto comp = component;
	visitor->Visit(comp->GetName(), MField(comp, X));
	visitor->Visit(comp->GetName(), MField(comp, Y));
	visitor->Visit(comp->GetName(), MField(comp, Z));
}

void Visit(PointLightComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->GetName();
	visitor->Visit(name, MField(comp, Color));
	visitor->Visit(name, MField(comp, Range));
	visitor->Visit(name, MField(comp, Intensity));
}

void Visit(DirectionalLightComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->GetName();
	visitor->Visit(name, MField(comp, Color));
	visitor->Visit(name, MField(comp, Intensity));
}
