#include "PostProcessDepthOfFieldStage.h"
#include "ShaderManager.h"
#include "InputLayout.h"
#include "Renderer.h"

struct BlurParams
{
	int Direction;
};

void PostProcessDepthOfFieldStage::Initialize()
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto vertexShaderBytecode = ShaderManager::LoadShader(L"QuadVS.cso");
	auto pixelShaderBytecode = ShaderManager::LoadShader(L"QuadPS.cso");

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	auto texFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;
	psoDesc.pRootSignature = renderer->GetDefaultRootSignature();
	psoDesc.VS = vertexShaderBytecode;
	psoDesc.PS = pixelShaderBytecode;
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
	auto screenSize = renderer->GetScreenSize();

	//Quarter res
	screenSize.Height /= 4;
	screenSize.Width /= 4;

	lowResTextureResource = ec->ResourceManager->CreateResource(
		CD3DX12_RESOURCE_DESC::Tex2D(texFormat, screenSize.Width, screenSize.Height,1,0,1,0,D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), 
		nullptr, D3D12_RESOURCE_STATE_RENDER_TARGET);
	auto texResource = ec->ResourceManager->GetResource(lowResTextureResource);
	lowResTexture = ec->ShaderResourceManager->CreateTexture(texResource);
	lowResRenderTarget = ec->RenderTargetManager->CreateRenderTargetView(texResource, texFormat);
	
	blurParamsCBV = ec->ShaderResourceManager->CreateCBV(sizeof(BlurParams));
}

TextureID PostProcessDepthOfFieldStage::RenderPostProcess(TextureID inputTexture)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();

	auto rtv = ec->RenderTargetManager->GetRTVHandle(lowResRenderTarget);
	float color[] = { 0,0,0,1 };

	auto screenSize = renderer->GetScreenSize();

	//Quarter res
	screenSize.Height /= 4;
	screenSize.Width /= 4;

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
	renderer->SetRenderTargets(&lowResRenderTarget, 1, nullptr);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	
	commandList->SetPipelineState(ec->ResourceManager->GetPSO(quadPso));

	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, renderer->GetCurrentRenderTargetTexture());
	renderer->DrawScreenQuad(commandList);

	return renderer->GetCurrentRenderTargetTexture();
}
