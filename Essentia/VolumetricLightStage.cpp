#include "pch.h"
#include "VolumetricLightStage.h"
#include "RenderTargetManager.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "Entity.h"
#include "PostProcessComponents.h"
#include "Mesh.h"
#include "PipelineStates.h"
#include "SceneResources.h"

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
		GContext->EntityManager->AddComponent<BaseDrawableComponent>(volumeEntity, BaseDrawableComponent::Create());
	}
	else
	{
		volumeEntity = entities[0];
	}

	MeshView meshView;
	cubeMesh = GContext->MeshManager->CreateMesh("Assets/Models/cube.obj", meshView);
	GRenderStageManager.RegisterStage("LightAccumulateStage", this);
}

void VolumetricLightStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto compManager = GContext->EntityManager->GetComponentManager();
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto rtManager = GContext->RenderTargetManager;
	auto sz = GPostProcess.GetPostSceneTextures().HalfResSize;

	uint32 count = 0;
	auto postProcessEntities = compManager->GetEntities<BaseDrawableComponent, PostProcessVolumeComponent>();
	auto baseDrawable = compManager->GetComponent<BaseDrawableComponent>(postProcessEntities[0]);

	renderer->TransitionBarrier(commandList, lightAccumTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(lightAccumTarget.RenderTarget), ColorValues::ClearColor, 0, nullptr);
	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.LightAccumPSO));
	renderer->SetTargetSize(commandList, sz);
	renderer->SetRenderTargets(&lightAccumTarget.RenderTarget, 1, &GSceneResources.DepthPrePass.DepthStencil);

	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, GSceneResources.LightBufferCBV);
	renderer->SetConstantBufferView(commandList, RootSigCBVertex0, baseDrawable->CBView);
	renderer->SetConstantBufferView(commandList, RootSigCBAll2, GSceneResources.ShadowCBV);
	renderer->SetShaderResourceView(commandList, RootSigSRVPixel2, GSceneResources.ShadowDepthTarget.Texture);
	renderer->DrawMesh(commandList, cubeMesh);

	renderer->TransitionBarrier(commandList, lightAccumTarget.Resource,  D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

}
