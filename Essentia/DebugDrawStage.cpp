#include "pch.h"
#include "DebugDrawStage.h"
#include "PipelineStates.h"
#include "Renderer.h"
#include "Entity.h"

using namespace DirectX;

void DebugDrawStage::Initialize()
{
	cubeMesh = GContext->MeshManager->GetMeshView("Assets/Models/cube.obj");
	GRenderStageManager.RegisterStage("DebugDrawStage", this);
}

void DebugDrawStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto modelManager = ec->ModelManager;
	auto sz = renderer->GetScreenSize();
	auto rtManager = GContext->RenderTargetManager;
	auto entityManager = GContext->EntityManager;

	renderer->SetPipelineState(commandList, GPipelineStates.WireframePSO);

	uint32 count = 0;
	auto entities = entityManager->GetEntities<SelectedComponent>(count);
	for (size_t i = 0; i < count; ++i)
	{
		auto drawable = frameContext.EntityManager->GetComponent<DrawableComponent>(entities[i]);
		if (drawable)
		{
			GContext->MeshManager->GetBoundingBox(drawable->Mesh);
			renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawable->CBView);
			renderer->DrawMesh(commandList, drawable->Mesh);
		}
		else
		{
			auto model = frameContext.EntityManager->GetComponent<DrawableModelComponent>(entities[i]);
			if (model)
			{
				renderer->SetConstantBufferView(commandList, RootSigCBVertex0, model->CBView);
				auto mesh = ec->ModelManager->GetModel(model->Model);
				renderer->DrawMesh(commandList, mesh.Mesh);
			}
		}
	}

	//for (uint32 i = 0; i < count; ++i)
	//{
	//	auto mesh = drawables[i].Mesh;
	//	renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawables[i].CBView);
	//	renderer->DrawMesh(commandList, mesh);
	//}

	//auto drawableModels = entityManager->GetComponents<DrawableModelComponent>(count);
	//for (uint32 i = 0; i < count; ++i)
	//{
	//	auto& model = modelManager->GetModel(drawableModels[i].Model);
	//	renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawableModels[i].CBView);
	//	auto meshHandle = model.Mesh;
	//	renderer->DrawMesh(commandList, meshHandle);
	//}

}

void DebugDrawStage::CleanUp()
{
}
