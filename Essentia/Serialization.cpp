#include "Interface.h"
#include "Serialization.h"
#include <functional>
#include <cereal/cereal.hpp>
#include "ComponentReflector.h"

ComponentReflector GComponentReflector;

void RegisterComponents()
{
	GComponentReflector.RegisterComponent<PositionComponent>();
	GComponentReflector.RegisterComponent<ScaleComponent>();
	GComponentReflector.RegisterComponent<RotationComponent>();
	GComponentReflector.RegisterComponent<PointLightComponent>();
	GComponentReflector.RegisterComponent<SpotLightComponent>();
	GComponentReflector.RegisterComponent<DirectionalLightComponent>();
	GComponentReflector.RegisterComponent<DrawableComponent>();
	GComponentReflector.RegisterComponent<DrawableModelComponent>();
	GComponentReflector.RegisterComponent<SkyboxComponent>();
	GComponentReflector.RegisterComponent<SelectedComponent>();
}

Scene LoadLevel(const char* fname)
{
	Scene scene = {};
	std::ifstream file(fname);
	cereal::JSONInputArchive archive(file);
	archive(cereal::make_nvp("Scene", scene));
	return scene;
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

void SaveScene(Scene&& scene, const char* fname)
{
	std::ofstream file(fname);
	cereal::JSONOutputArchive archive(file);
	archive(cereal::make_nvp("Scene", scene));
}

template<class Archive>
void serialize(Archive& archive, EntityHandle& entity)
{
	archive(CEREAL_NVP(entity.ID));
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

void Visit(SpotLightComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->GetName();
	visitor->Visit(name, MField(comp, Color));
	visitor->Visit(name, MField(comp, Range));
	visitor->Visit(name, MField(comp, Intensity));
	visitor->Visit(name, MField(comp, Direction));
	visitor->Visit(name, MField(comp, SpotFalloff));
}

void Visit(DirectionalLightComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->GetName();
	visitor->Visit(name, MField(comp, Color));
	visitor->Visit(name, MField(comp, Intensity));
	visitor->Visit(name, MField(comp, Direction));
}

void Visit(DrawableComponent* component, IVisitor* visitor)
{
}

void Visit(DrawableModelComponent* component, IVisitor* visitor)
{
}

void Visit(SkyboxComponent* component, IVisitor* visitor)
{
}

void Visit(SelectedComponent* component, IVisitor* visitor)
{
}
