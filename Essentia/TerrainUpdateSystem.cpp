#include "TerrainUpdateSystem.h"
#include "TerrainComponent.h"
#include "Entity.h"
#include "EngineContext.h"
#include "Renderer.h"

using namespace DirectX;

void TerrainUpdateSystem::Initialize()
{
}

void TerrainUpdateSystem::Update(float dt, float totalTime)
{
	EntityManager* entityManager = GContext->EntityManager;
	TerrainManager* terrainManager = GContext->TerrainManager;

	uint32 count = 0;
	auto terrains = entityManager->GetComponents<TerrainComponent>(count);

	// Update terrain mesh if required
	for (uint32 i = 0; i < count; ++i)
	{
		// To do - ImGui should trigger an event when there is some change
		if (terrains[i].PrevMaxY != terrains[i].ScaleMaxY || terrains[i].PrevMinY != terrains[i].ScaleMinY)
		{
			terrainManager->UpdateTerrainMesh(terrains[i].TerrainName.c_str(), terrains[i].ScaleMinY, terrains[i].ScaleMaxY);
		}

		terrains[i].PrevMaxY = terrains[i].ScaleMaxY;
		terrains[i].PrevMinY = terrains[i].ScaleMinY;
	}

	auto camera = entityManager->GetComponents<CameraComponent>(count)[0].CameraInstance;
	auto view = XMLoadFloat4x4(&camera.View);
	auto projection = XMLoadFloat4x4(&camera.Projection);

	auto entities = entityManager->GetEntities<TerrainComponent>(count);
	uint32 frameIndex = GContext->RendererInstance->GetCurrentBackbufferIndex();
	for (uint32 i = 0; i < count; ++i)
	{
		XMFLOAT4X4 worldMatrix = entityManager->GetWorldMatrix(entities[i]);
		auto world = XMLoadFloat4x4(&worldMatrix);
		TerrainComponent* terrain = entityManager->GetComponent<TerrainComponent>(entities[i]);

		//Update Constant Buffer Values
		terrain->ConstantBuffer.PrevWorldViewProjection = terrain->ConstantBuffer.WorldViewProjection;
		XMStoreFloat4x4(&terrain->ConstantBuffer.WorldViewProjection, XMMatrixTranspose(world * view * projection));
		XMStoreFloat4x4(&terrain->ConstantBuffer.World, XMMatrixTranspose(world));
		terrain->ConstantBuffer.View = camera.GetViewTransposed();
		terrain->ConstantBuffer.Projection = camera.GetProjectionTransposed();
		GContext->ShaderResourceManager->CopyToCB(frameIndex, { &terrains[i].ConstantBuffer, sizeof(terrains[i].ConstantBuffer) }, terrains[i].ConstantBufferView);
	}
}
