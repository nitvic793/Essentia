#include "PostProcessDepthOfFieldStage.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "Renderer.h"
#include "PostProcess.h"

//Do AO?

struct BlurParams
{
	int		Direction; // 0-Horizontal, 1-Vertical
	float	Width;
	float	Height;
	float	Padding;
};

void PostProcessDepthOfFieldStage::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	downscaleFactor = 1;
	auto texFormat = renderer->GetRenderTargetFormat();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = ShaderManager::LoadShader(L"QuadVS.cso");
	psoDesc.PS = ShaderManager::LoadShader(L"QuadPS.cso");
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

	quadPso = ec->ResourceManager->CreatePSO(psoDesc);

	psoDesc.PS = ShaderManager::LoadShader(L"BlurPS.cso");
	blurPso = ec->ResourceManager->CreatePSO(psoDesc);

	auto screenSize = renderer->GetScreenSize();

	//Quarter res
	screenSize.Height /= downscaleFactor;
	screenSize.Width /= downscaleFactor;

	lowResTarget = CreatePostProcessRenderTarget(ec, screenSize.Width, screenSize.Height, texFormat);

	blurHorizontalCBV = ec->ShaderResourceManager->CreateCBV(sizeof(BlurParams));
	blurVerticalCBV = ec->ShaderResourceManager->CreateCBV(sizeof(BlurParams));
	blurIntermidateTarget = CreatePostProcessRenderTarget(ec, screenSize.Width, screenSize.Height, texFormat);
	blurFinalTarget = CreatePostProcessRenderTarget(ec, screenSize.Width, screenSize.Height, texFormat);
}

TextureID PostProcessDepthOfFieldStage::RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture)
{

	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	renderer->TransitionBarrier(commandList, lowResTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtv = ec->RenderTargetManager->GetRTVHandle(lowResTarget.RenderTarget);
	float color[] = { 0,0,0,1 };

	auto screenSize = renderer->GetScreenSize();

	//Quarter res
	screenSize.Height /= downscaleFactor;
	screenSize.Width /= downscaleFactor;

	D3D12_VIEWPORT viewport = {};

	viewport.Width = (FLOAT)screenSize.Width;
	viewport.Height = (FLOAT)screenSize.Height;

	D3D12_RECT scissorRect = {};

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = screenSize.Width;
	scissorRect.bottom = screenSize.Height;

	commandList->SetGraphicsRootSignature(renderer->GetDefaultRootSignature());
	commandList->ClearRenderTargetView(rtv, color, 0, nullptr);
	renderer->SetRenderTargets(&lowResTarget.RenderTarget, 1, nullptr);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->SetPipelineState(ec->ResourceManager->GetPSO(quadPso));

	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, renderer->GetCurrentRenderTargetTexture());
	renderer->DrawScreenQuad(commandList);

	BlurParams params = { 0, (float)screenSize.Width, (float)screenSize.Height };

	ec->ShaderResourceManager->CopyToCB(backbufferIndex, { &params, sizeof(params) }, blurHorizontalCBV.Offset);
	params.Direction = 1;
	ec->ShaderResourceManager->CopyToCB(backbufferIndex, { &params, sizeof(params) }, blurVerticalCBV.Offset);

	TransitionDesc transitions[] = {
		{ lowResTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
		{  blurIntermidateTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET }
	};

	renderer->TransitionBarrier(commandList, transitions, 2);

	auto blurRtv = ec->RenderTargetManager->GetRTVHandle(blurIntermidateTarget.RenderTarget);
	commandList->ClearRenderTargetView(blurRtv, color, 0, nullptr);
	renderer->SetRenderTargets(&blurIntermidateTarget.RenderTarget, 1, nullptr);

	commandList->SetPipelineState(ec->ResourceManager->GetPSO(blurPso));
	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, lowResTarget.Texture);
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, blurHorizontalCBV);
	renderer->DrawScreenQuad(commandList);

	TransitionDesc blurTransitions[] = {
		{ blurIntermidateTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
		{ blurFinalTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET }
	};

	renderer->TransitionBarrier(commandList, blurTransitions, 2);
	blurRtv = ec->RenderTargetManager->GetRTVHandle(blurFinalTarget.RenderTarget);
	renderer->SetRenderTargets(&blurFinalTarget.RenderTarget, 1, nullptr);
	commandList->ClearRenderTargetView(blurRtv, color, 0, nullptr);

	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, blurIntermidateTarget.Texture);
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, blurVerticalCBV);
	renderer->DrawScreenQuad(commandList);


	renderer->TransitionBarrier(commandList, blurFinalTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	renderer->TransitionBarrier(commandList, renderer->GetCurrentRenderTargetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	screenSize = renderer->GetScreenSize();

	auto rt = renderer->GetDefaultRenderTarget();
	renderer->SetRenderTargets(&rt, 1, nullptr);

	viewport.Width = (FLOAT)screenSize.Width;
	viewport.Height = (FLOAT)screenSize.Height;
	scissorRect.right = screenSize.Width;
	scissorRect.bottom = screenSize.Height;

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetPipelineState(ec->ResourceManager->GetPSO(quadPso));
	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, blurFinalTarget.Texture);
	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, renderer->GetCurrentRenderTargetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	return renderer->GetCurrentRenderTargetTexture();
}
