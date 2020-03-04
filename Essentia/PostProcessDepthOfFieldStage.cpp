#include "PostProcessDepthOfFieldStage.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "Renderer.h"
#include "PostProcess.h"
#include "PipelineStates.h"
#include "Entity.h"
//Do AO?
using namespace DirectX;

void PostProcessDepthOfFieldStage::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	auto texFormat = renderer->GetHDRRenderTargetFormat();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"QuadVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"BlurPS.cso");
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

	psoDesc.PS = ShaderManager::LoadShader(L"DepthOfFieldPS.cso");
	dofPso = ec->ResourceManager->CreatePSO(psoDesc);

	blurHorizontalCBV = ec->ShaderResourceManager->CreateCBV(sizeof(BlurParams));
	blurVerticalCBV = ec->ShaderResourceManager->CreateCBV(sizeof(BlurParams));
	dofCBV = ec->ShaderResourceManager->CreateCBV(sizeof(DepthOfFieldParams));

	auto halfRes = GPostProcess.GetPostSceneTextures().HalfResSize;
	auto fullRes = renderer->GetScreenSize();
	blurIntermidateTarget = CreateSceneRenderTarget(ec, halfRes.Width, halfRes.Height, texFormat);
	blurFinalTarget = CreateSceneRenderTarget(ec, halfRes.Width, halfRes.Height, texFormat);
	dofTarget = CreateSceneRenderTarget(ec, fullRes.Width, fullRes.Height, texFormat);
	DofParams = { 0, 0, 5.f, 0.90f };
	GPostProcess.RegisterPostProcess("DepthOfField", this);
}

TextureID PostProcessDepthOfFieldStage::RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto em = ec->EntityManager;
	auto commandList = renderer->GetDefaultCommandList();
	auto inputResource = ec->ShaderResourceManager->GetResource(inputTexture);
	auto screenSize = renderer->GetScreenSize();
	uint32 count = 0;
	auto camera = &em->GetComponents<CameraComponent>(count)[0].CameraInstance;

	RenderBlurTexture(backbufferIndex);

	renderer->TransitionBarrier(commandList, dofTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	DofParams.zNear = camera->NearZ;
	DofParams.zFar = camera->FarZ;

	ec->ShaderResourceManager->CopyToCB(backbufferIndex, { &DofParams, sizeof(DofParams) }, dofCBV.Offset);
	auto dofRtv = ec->RenderTargetManager->GetRTVHandle(dofTarget.RenderTarget);
	commandList->ClearRenderTargetView(dofRtv, ColorValues::ClearColor, 0, nullptr);
	renderer->SetTargetSize(commandList, screenSize);

	TextureID textures[] = { inputTexture, blurFinalTarget.Texture, renderer->GetCurrentDepthStencilTexture() };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, dofCBV);
	renderer->SetRenderTargets(&dofTarget.RenderTarget, 1, nullptr);
	commandList->SetPipelineState(ec->ResourceManager->GetPSO(dofPso));
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, dofTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return dofTarget.Texture;
}

void PostProcessDepthOfFieldStage::RenderBlurTexture(uint32 backbufferIndex)
{
	auto lowResTextures = GPostProcess.GetPostSceneTextures();
	auto screenSize = lowResTextures.HalfResSize;
	auto lowResTarget = GPostProcess.GetPostSceneTextures().HalfResTexture;
	GPostProcess.RenderBlurTexture(lowResTarget.Texture, screenSize, backbufferIndex, blurFinalTarget, blurIntermidateTarget, blurVerticalCBV, blurHorizontalCBV);
}
