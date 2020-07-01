#pragma once
#include "RenderStage.h"

static constexpr uint32 CVoxelGridMips = 8;

static const VoxelParams CreateVoxelParams(const Camera& camera, uint32 voxelSize)
{
	using namespace DirectX;

	VoxelParams output = {};
	XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
	camera.Frustum.GetCorners(corners);
	BoundingBox cameraAABB;
	BoundingBox::CreateFromPoints(cameraAABB, camera.Frustum.CORNER_COUNT, corners, sizeof(XMFLOAT3));

	XMVECTOR voxelGridCenter = XMLoadFloat3(&cameraAABB.Center);

	XMStoreFloat3(&output.VoxelGridCenter, voxelGridCenter);
	output.VoxelRadianceDataSize = 1.f;
	output.VoxelRadianceDataRes = CVoxelSize;
	output.VoxelRadianceDataSizeRCP = 1.f / output.VoxelRadianceDataSize;
	output.VoxelRadianceDataResRCP = 1.f / CVoxelSize;

	const float f = 0.05f / output.VoxelRadianceDataSize;
	XMFLOAT3 center = XMFLOAT3(floorf(camera.Position.x * f) / f, floorf(camera.Position.y * f) / f, floorf(camera.Position.z * f) / f);

	output.VoxelGridCenter = center;
	output.VoxelRadianceNumCones = 2;
	output.VoxelRadianceMaxDistance = 100.f;
	output.VoxelRadianceRayStepSize = 0.75f;
	output.VoxelRadianceNumConesRCP = 1.f / (float)output.VoxelRadianceNumCones;
	output.VoxelRadianceMips = CVoxelGridMips;

	return output;
}

class VoxelizationStage :
	public IRenderStage
{
public:
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void Initialize() override;
private:
	TextureID			voxelGrid3dTextureSRV;
	TextureID			voxelGrid3dTextureUAV;
	ResourceID			voxelGridResource;
	SceneRenderTarget	voxelRT;
	ConstantBufferView	voxelParamsCBV;
	VoxelParams			voxelParams;
};

