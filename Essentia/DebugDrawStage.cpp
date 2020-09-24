#include "pch.h"
#include "DebugDrawStage.h"
#include "PipelineStates.h"
#include "Renderer.h"
#include "Entity.h"
#include <EffectPipelineStateDescription.h>
#include <CommonStates.h>
#include <Effects.h>
#include <memory>
#include <DirectXColors.h>
#include "DebugDraw.h"
using namespace DirectX;



void DebugDrawStage::Initialize()
{
	cubeMesh = GContext->MeshManager->GetMeshView("Assets/Models/cube.obj");
	GRenderStageManager.RegisterStage("DebugDrawStage", this);

	mBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(GContext->DeviceResources->GetDevice());
	RenderTargetState rtState(GContext->RendererInstance->GetRenderTargetFormat(), GContext->RendererInstance->GetDepthStencilFormat());
	EffectPipelineStateDescription pd(
		&VertexPositionColor::InputLayout,
		CommonStates::Opaque,
		CommonStates::DepthDefault,
		CommonStates::CullNone,
		rtState);
	pd.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	mEffect = std::unique_ptr<BasicEffect>(new BasicEffect(GContext->DeviceResources->GetDevice(), EffectFlags::VertexColor, pd));
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
	static BoundingBox box;
	renderer->SetPipelineState(commandList, GPipelineStates.WireframePSO);

	uint32 count = 0;
	auto entities = entityManager->GetEntities<SelectedComponent>(count);
	
	for (size_t i = 0; i < count; ++i)
	{
		auto drawable = frameContext.EntityManager->GetComponent<DrawableComponent>(entities[i]);
		if (drawable)
		{
			auto world = GContext->EntityManager->GetWorldMatrix(entities[i]);
			box = GContext->MeshManager->GetBoundingBox(drawable->Mesh);
			box.Transform(box, XMLoadFloat4x4(&world));
			if (drawable->IsAnimated())continue;
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
				auto world = GContext->EntityManager->GetWorldMatrix(entities[i]);
				box = GContext->MeshManager->GetBoundingBox(mesh.Mesh);
				box.Transform(box, XMLoadFloat4x4(&world));
			}
		}
	}

	auto compManager = GContext->EntityManager->GetComponentManager();
	const auto& camera = compManager->GetAllComponents<CameraComponent>(count)[0].CameraInstance;

	mEffect->SetView(XMLoadFloat4x4(&camera.View));
	mEffect->SetProjection(XMLoadFloat4x4(&camera.Projection));
	mEffect->Apply(commandList);

	mBatch->Begin(commandList);
	Draw(mBatch.get(), box, Colors::Blue); // BoundingBox

	mBatch->End();

	commandList->SetGraphicsRootSignature(renderer->GetDefaultRootSignature());
	commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DebugDrawStage::CleanUp()
{
}
