#include "pch.h"
#include "PostProcess.h"
#include "RenderTargetManager.h"



PostProcessRenderTarget CreatePostProcessRenderTarget(EngineContext* context, uint32 width, uint32 height, DXGI_FORMAT format)
{
	PostProcessRenderTarget target;
	auto ec = context;
	target.Resource = ec->ResourceManager->CreateResource(
		CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
		nullptr, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	auto texResource = ec->ResourceManager->GetResource(target.Resource);
	target.Texture = ec->ShaderResourceManager->CreateTexture(texResource);
	target.RenderTarget = ec->RenderTargetManager->CreateRenderTargetView(texResource, format);
	return target;
}