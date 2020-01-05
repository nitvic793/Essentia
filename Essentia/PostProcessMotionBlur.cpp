#include "pch.h"
#include "PostProcessMotionBlur.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "SceneResources.h"

using namespace DirectX;

void PostProcessMotionBlur::Initialize()
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
	psoDesc.VS = ShaderManager::LoadShader(L"QuadVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"MotionBlurPS.cso");
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

	motionBlurPso = resourceManager->CreatePSO(psoDesc);
	GPostProcess.RegisterPostProcess("MotionBlur", this);
	motionBlurTarget = CreateSceneRenderTarget(ec, size.Width, size.Height, texFormat);
	motionCBV = ec->ShaderResourceManager->CreateCBV(sizeof(MotionBlurParams));
}

TextureID PostProcessMotionBlur::RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto resourceManager = ec->ResourceManager;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto screenSize = renderer->GetScreenSize();

	const float targetFps = 60.f;
	float fps = 1.f / frameContext.Timer->DeltaTime;

	MotionBlurParams params = {};
	params.ScreenSize = XMFLOAT2((float)screenSize.Width, (float)screenSize.Height);
	params.VelocityScale = fps / targetFps;
	ec->ShaderResourceManager->CopyToCB(backbufferIndex, { &params, sizeof(params) }, motionCBV.Offset);

	auto rt = ec->RenderTargetManager->GetRTVHandle(motionBlurTarget.RenderTarget);
	renderer->TransitionBarrier(commandList, motionBlurTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	renderer->SetTargetSize(commandList, screenSize);
	commandList->ClearRenderTargetView(rt, ColorValues::ClearColor, 0, nullptr);
	renderer->SetRenderTargets(&motionBlurTarget.RenderTarget, 1, nullptr);
	commandList->SetPipelineState(resourceManager->GetPSO(motionBlurPso));

	TextureID textures[] = { inputTexture, GSceneResources.VelocityBuffer.Texture };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, motionCBV);

	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, motionBlurTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return motionBlurTarget.Texture;
}
