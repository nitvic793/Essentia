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

struct LightAccumParams
{
	DirectX::XMFLOAT4X4 InvProjection;
	DirectX::XMFLOAT4X4 World;
};

void VolumetricLightStage::Initialize()
{
	auto renderer = GContext->RendererInstance;
	auto entityManager = GContext->EntityManager;
	auto halfResSize = GPostProcess.GetPostSceneTextures().HalfResSize;
	lightAccumTarget = CreateSceneRenderTarget(GContext, halfResSize.Width, halfResSize.Height, renderer->GetHDRRenderTargetFormat());
	GSceneResources.LightAccumTarget = lightAccumTarget;
	uint32 count = 0;
	auto entities = entityManager->GetEntities<PostProcessVolumeComponent>(count);
	if (count == 0)
	{
		volumeEntity = GContext->EntityManager->CreateEntity();
		GContext->EntityManager->AddComponent<PostProcessVolumeComponent>(volumeEntity);
		GContext->EntityManager->AddComponent<BaseDrawableComponent>(volumeEntity, BaseDrawableComponent::Create());
		auto transform = GContext->EntityManager->GetTransform(volumeEntity);
		//*transform.Scale = DirectX::XMFLOAT3(100.f, 100.f, 100.f);
	}
	else
	{
		volumeEntity = entities[0];
	}

	lightAccumCBV = GContext->ShaderResourceManager->CreateCBV(sizeof(LightAccumParams));
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
	uint32 count;
	const auto& camera = compManager->GetAllComponents<CameraComponent>(count)[0].CameraInstance;

	auto projection = DirectX::XMLoadFloat4x4(&camera.Projection);
	auto invProjection = DirectX::XMMatrixInverse(nullptr, projection);
	LightAccumParams params;
	DirectX::XMStoreFloat4x4(&params.InvProjection, DirectX::XMMatrixTranspose(invProjection));

	auto postProcessEntities = compManager->GetEntities<BaseDrawableComponent, PostProcessVolumeComponent>();
	auto baseDrawable = compManager->GetComponent<BaseDrawableComponent>(postProcessEntities[0]);
	params.World = baseDrawable->World;
	shaderResourceManager->CopyToCB(frameIndex, { &params, sizeof(params) }, lightAccumCBV);
	renderer->TransitionBarrier(commandList, lightAccumTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(lightAccumTarget.RenderTarget), ColorValues::ClearColor, 0, nullptr);
	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.LightAccumPSO));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)sz.Width;
	viewport.Height = (FLOAT)sz.Height;
	viewport.MaxDepth = 1.f;
	viewport.MinDepth = 0.f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = sz.Width;
	scissorRect.bottom = sz.Height;

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	renderer->SetRenderTargets(&lightAccumTarget.RenderTarget, 1, nullptr);

	TextureID textures[] = { GSceneResources.ShadowDepthTarget.Texture, GSceneResources.DepthPrePass.Texture, GSceneResources.NoiseTexture, GSceneResources.WorldPosTexture.Texture };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel2, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, GSceneResources.LightBufferCBV);
	renderer->SetConstantBufferView(commandList, RootSigCBVertex0, baseDrawable->CBView);
	renderer->SetConstantBufferView(commandList, RootSigCBAll2, GSceneResources.ShadowCBV);
	renderer->SetConstantBufferView(commandList, RootSigCBAll1, lightAccumCBV);

	renderer->DrawScreenQuad(commandList);
	//renderer->DrawMesh(commandList, cubeMesh);

	renderer->TransitionBarrier(commandList, lightAccumTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

}
