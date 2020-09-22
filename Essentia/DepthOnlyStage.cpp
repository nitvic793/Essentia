#include "pch.h"
#include "DepthOnlyStage.h"
#include "Entity.h"
#include "Renderer.h"
#include "PipelineStates.h"
#include "Mesh.h"
#include "SceneResources.h"
#include "AnimationComponent.h"

void DepthOnlyStage::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = GContext->RendererInstance;
	auto sz = renderer->GetScreenSize();
	depthOnlyPso = ec->ResourceManager->GetPSO(GPipelineStates.DepthOnlyPSO);
	depthTarget = CreateDepthTarget(sz.Width, sz.Height, renderer->GetDepthStencilFormat(), DXGI_FORMAT_R32_FLOAT);
	worldPosTarget = CreateSceneRenderTarget(GContext, sz.Width, sz.Height, renderer->GetHDRRenderTargetFormat());
	GSceneResources.DepthPrePass = depthTarget;
	GSceneResources.WorldPosTexture = worldPosTarget;
	GRenderStageManager.RegisterStage("DepthOnlyStage", this);
}

void DepthOnlyStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto modelManager = ec->ModelManager;
	auto sz = renderer->GetScreenSize();
	auto rtManager = GContext->RenderTargetManager;
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

	TransitionDesc transitions[] = {
		{ depthTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE },
		{ worldPosTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET }
	};

	renderer->TransitionBarrier(commandList, transitions, _countof(transitions));

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	renderer->SetRenderTargets(nullptr, 0, &depthTarget.DepthStencil);
	commandList->SetPipelineState(depthOnlyPso);
	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(worldPosTarget.RenderTarget), ColorValues::ClearColor, 0, nullptr);
	commandList->ClearDepthStencilView(rtManager->GetDSVHandle(depthTarget.DepthStencil), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
	uint32 count = 0;
	auto drawables = frameContext.EntityManager->GetComponents<DrawableComponent>(count);

	for (uint32 i = 0; i < count; ++i)
	{
		if (drawables[i].IsAnimated())continue;
		auto mesh = drawables[i].Mesh;
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawables[i].CBView);
		renderer->DrawMesh(commandList, mesh);
	}

	auto drawableModels = frameContext.EntityManager->GetComponents<DrawableModelComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		auto& model = modelManager->GetModel(drawableModels[i].Model);
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawableModels[i].CBView);
		auto meshHandle = model.Mesh;
		renderer->DrawMesh(commandList, meshHandle);
	}

	renderer->SetPipelineState(commandList, GPipelineStates.DepthOnlyAnimatedPSO);
	auto entites = frameContext.EntityManager->GetEntities<AnimationComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		auto drawable = GContext->EntityManager->GetComponent<DrawableComponent>(entites[i]);
		auto animComponent = GContext->EntityManager->GetComponent<AnimationComponent>(entites[i]);
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawable->CBView);
		renderer->SetConstantBufferView(commandList, RootSigCBAll1, animComponent->ArmatureCBV);
		renderer->DrawAnimatedMesh(commandList, GContext->MeshManager->GetMeshView(drawable->Mesh));
	}

	TransitionDesc endTransitions[] = {
		{ depthTarget.Resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
		{ worldPosTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE }
	};

	renderer->TransitionBarrier(commandList, endTransitions, _countof(endTransitions));
}
