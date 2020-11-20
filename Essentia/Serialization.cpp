#include "Interface.h"
#include "Serialization.h"
#include <functional>
#include <cereal/cereal.hpp>
#include "ComponentReflector.h"
#include "PostProcessComponents.h"

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
	GComponentReflector.RegisterComponent<PostProcessVolumeComponent>();
	GComponentReflector.RegisterComponent<BaseDrawableComponent>();
	GComponentReflector.RegisterComponent<CameraComponent>();
	GComponentReflector.RegisterComponent<AnimationComponent>();
	GComponentReflector.RegisterComponent<BoundingOrientedBoxComponent>();
	GComponentReflector.RegisterComponent<TerrainComponent>();
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
	auto name = comp->ComponentName;
	visitor->Visit(name, MField(comp, X));
	visitor->Visit(name, MField(comp, Y));
	visitor->Visit(name, MField(comp, Z));
}

void Visit(RotationComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	visitor->Visit(name, MField(comp, X));
	visitor->Visit(name, MField(comp, Y));
	visitor->Visit(name, MField(comp, Z));
}

void Visit(ScaleComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	visitor->Visit(name, MField(comp, X));
	visitor->Visit(name, MField(comp, Y));
	visitor->Visit(name, MField(comp, Z));
}

void Visit(PointLightComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	visitor->Visit(name, MField(comp, Color));
	visitor->Visit(name, MField(comp, Range));
	visitor->Visit(name, MField(comp, Intensity));
}

void Visit(SpotLightComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	visitor->Visit(name, MField(comp, Color));
	visitor->Visit(name, MField(comp, Range));
	visitor->Visit(name, MField(comp, Intensity));
	visitor->Visit(name, MField(comp, Direction));
	visitor->Visit(name, MField(comp, SpotFalloff));
}

void Visit(DirectionalLightComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	visitor->Visit(name, MField(comp, Color));
	visitor->Visit(name, MField(comp, Intensity));
	visitor->Visit(name, MField(comp, Direction));
}

void Visit(DrawableComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	visitor->Visit(name, MField(comp, Mesh));
	visitor->Visit(name, MField(comp, Material));
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

void Visit(PostProcessVolumeComponent* component, IVisitor* visitor)
{
}

void Visit(BaseDrawableComponent* component, IVisitor* visitor)
{
}

void Visit(CameraComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	auto Cam = &comp->CameraInstance;
	visitor->Visit(name, MField(Cam, Width));
	visitor->Visit(name, MField(Cam, Height));
	visitor->Visit(name, MField(Cam, NearZ));
	visitor->Visit(name, MField(Cam, FarZ));
	visitor->Visit(name, MField(Cam, FieldOfView));
	visitor->Visit(name, MField(Cam, IsOrthographic));
}

void Visit(AnimationComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	visitor->Visit(name, MField(comp, CurrentAnimationIndex));
	visitor->Visit(name, MField(comp, CurrentAnimation));
	visitor->Visit(name, MField(comp, IsPlaying));
	visitor->Visit(name, MField(comp, AnimationSpeed));
}

void Visit(BoundingOrientedBoxComponent* component, IVisitor* visitor)
{
}

void Visit(TerrainComponent* component, IVisitor* visitor)
{
	auto comp = component;
	auto name = comp->ComponentName;
	std::string_view terrainName = std::string_view(comp->TerrainName.c_str());
	visitor->Visit(name, MField(comp, ScaleMaxY));
	visitor->Visit(name, MField(comp, ScaleMinY));
	visitor->Visit(name, "TerrainName", terrainName);
}
