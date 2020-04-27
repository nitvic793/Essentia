#include "pch.h"
#include "VoxelizationStage.h"


void VoxelizationStage::Initialize()
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	uint32 voxelGridSize = 128;
	Texture3DCreateProperties props = {};
	props.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	props.Width = voxelGridSize;
	props.Depth = voxelGridSize;
	props.Height = voxelGridSize;
	props.MipLevels = 1;

	ResourceID resource;
	voxelGrid3dTextureSRV = shaderResourceManager->CreateTexture3D(props, &resource, nullptr, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	voxelGrid3dTextureUAV = shaderResourceManager->CreateTexture3DUAV(resource, voxelGridSize);
	GRenderStageManager.RegisterStage("VoxelizationStage", this);
}

void VoxelizationStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{

}


