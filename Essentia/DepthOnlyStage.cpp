#include "pch.h"
#include "DepthOnlyStage.h"
#include "Entity.h"
#include "Renderer.h"
#include "PipelineStates.h"
#include "Mesh.h"

void DepthOnlyStage::Initialize()
{
	auto ec = EngineContext::Context;
	depthOnlyPso = ec->ResourceManager->GetPSO(GPipelineStates.DepthOnlyPSO);
	GRenderStageManager.RegisterStage("DepthOnlyStage", this);
}

void DepthOnlyStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto modelManager = ec->ModelManager;

	renderer->SetTargetSize(commandList, renderer->GetScreenSize());
	auto depthStencil = renderer->GetCurrentDepthStencil();
	renderer->SetRenderTargets(nullptr, 0, &depthStencil);
	commandList->SetPipelineState(depthOnlyPso);

	uint32 count = 0;
	auto drawables = frameContext.EntityManager->GetComponents<DrawableComponent>(count);

	for (uint32 i = 0; i < count; ++i)
	{
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
}
