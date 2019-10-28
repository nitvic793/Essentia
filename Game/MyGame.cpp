#include "pch.h"
#include "MyGame.h"

#include "System.h"
#include "Entity.h"
#include <DirectXMath.h>

using namespace DirectX;

void InitializeResources()
{
	auto ec = EngineContext::Context;
	auto entityManager = ec->EntityManager;
	auto entity = entityManager->CreateEntity(); //0
	auto entity2 = entityManager->CreateEntity(); //1
	auto e = entityManager->CreateEntity(); //2

	EntityHandle lights[2];

	lights[0] = entityManager->CreateEntity();//3
	lights[1] = entityManager->CreateEntity();//4
	auto skybox = entityManager->CreateEntity();//5
	entityManager->AddComponent<SkyboxComponent>(skybox, SkyboxComponent::Create("../../Assets/Textures/SunnyCubeMap.dds"));
	XMFLOAT3 direction;
	auto dir = XMVector3Normalize(XMVectorSet(1, -1, 1, 0));
	XMStoreFloat3(&direction, dir);
	entityManager->AddComponent<DirectionalLightComponent>(lights[0], DirectionalLightComponent::Create(direction, XMFLOAT3(0.9f, 0.9f, 0.9f)));

	entityManager->AddComponent<PointLightComponent>(lights[1], PointLightComponent::Create(XMFLOAT3(0.9f, 0.1f, 0.1f)));
	auto transform = entityManager->GetTransform(lights[1]);
	transform.Position->y = 3;
	MaterialHandle mat = { 0 };
	MeshHandle mesh = { 1 };
	MeshHandle cone = Es::CreateMesh("../../Assets/Models/cube.obj");

	entityManager->AddComponent<DrawableComponent>(entity, DrawableComponent::Create(mesh, mat));
	entityManager->AddComponent<DrawableComponent>(entity2, DrawableComponent::Create(cone, mat));
	entityManager->AddComponent<DrawableModelComponent>(e, DrawableModelComponent::Create({ 0 }));

	entityManager->AddComponent<SelectedComponent>(entity);
	transform = entityManager->GetTransform(e);
	transform.Position->z = 4;
	auto scale = XMFLOAT3(0.05f, 0.05f, 0.05f);
	memcpy(transform.Scale, &scale, sizeof(scale));

	transform = entityManager->GetTransform(entity2);
	transform.Position->z = 5;
}
