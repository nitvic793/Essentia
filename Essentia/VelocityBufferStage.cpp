#include "pch.h"
#include "VelocityBufferStage.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "Renderer.h"
#include "SceneResources.h"
#include "Entity.h"
#include "pix3.h"

void VelocityBufferStage::Initialize()
{
	auto ec = EngineContext::Context;
	auto resourceManager = ec->ResourceManager;
	auto renderer = ec->RendererInstance;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	auto texFormat = renderer->GetHDRRenderTargetFormat();
	auto size = renderer->GetScreenSize();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	psoDesc.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"VelocityBufferVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"VelocityBufferPS.cso");
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = texFormat;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.DepthClipEnable = false;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;

	velocityBufferPSO = resourceManager->CreatePSO(psoDesc);
	GPostProcess.RegisterPostProcess("VelocityBufferStage", this);
}

TextureID VelocityBufferStage::RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto resourceManager = ec->ResourceManager;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto screenSize = renderer->GetScreenSize();
	auto rt = ec->RenderTargetManager->GetRTVHandle(GSceneTextures.VelocityBuffer.RenderTarget);

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Velocity Buffer");
	renderer->TransitionBarrier(commandList, GSceneTextures.VelocityBuffer.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	renderer->SetTargetSize(commandList, screenSize);
	commandList->ClearRenderTargetView(rt, ColorValues::ClearColor, 0, nullptr);
	renderer->SetRenderTargets(&GSceneTextures.VelocityBuffer.RenderTarget, 1, nullptr);
	commandList->SetPipelineState(resourceManager->GetPSO(velocityBufferPSO));
	uint32 count;
	auto drawables = ec->EntityManager->GetComponents<DrawableComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawables[i].CBView);
		renderer->DrawMesh(drawables[i].Mesh);
	}

	//auto drawableModels = ec->EntityManager->GetComponents<DrawableModelComponent>(count);

	//for (uint32 i = 0; i < count; ++i)
	//{
	//	renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawableModels[i].CBView);
	//	auto model = ec->ModelManager->GetModel(drawableModels[i].Model);
	//	renderer->DrawMesh(model.Mesh);
	//}

	renderer->TransitionBarrier(commandList, GSceneTextures.VelocityBuffer.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	PIXEndEvent(commandList);
	return inputTexture;
}

