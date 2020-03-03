#include "pch.h"
#include "VolumetricLightStage.h"
#include "RenderTargetManager.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "Entity.h"
#include "PostProcessComponents.h"
#include "Mesh.h"
#include "PipelineStates.h"

void VolumetricLightStage::Initialize()
{
	auto renderer = GContext->RendererInstance;
	auto entityManager = GContext->EntityManager;
	auto halfResSize = GPostProcess.GetPostSceneTextures().HalfResSize;
	lightAccumTarget = CreateSceneRenderTarget(GContext, halfResSize.Width, halfResSize.Height, renderer->GetHDRRenderTargetFormat());
	uint32 count = 0;
	auto entities = entityManager->GetEntities<PostProcessVolumeComponent>(count);
	if (count == 0)
	{
		volumeEntity = GContext->EntityManager->CreateEntity();
		GContext->EntityManager->AddComponent<PostProcessVolumeComponent>(volumeEntity);
		GContext->EntityManager->AddComponent<BaseDrawableComponent>(volumeEntity);
	}
	else
	{
		volumeEntity = entities[0];
	}

	MeshView meshView;
	cubeMesh = GContext->MeshManager->CreateMesh("Assets/Models/cube.obj", meshView);
}

void VolumetricLightStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto rtManager = GContext->RenderTargetManager;
	auto sz = GPostProcess.GetPostSceneTextures().HalfResSize;

	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.LightAccumPSO));


}
