#pragma once

#include <DirectXMath.h>

constexpr uint32_t CVoxelSize = 128;
constexpr uint32_t CMaxDirLights = 4;
constexpr uint32_t CMaxPointLights = 64;
constexpr uint32_t CMaxSpotLights = 16;
constexpr uint32_t CMaxBones = 128;

struct DECLSPEC_ALIGN(16) VoxelParams
{
	DirectX::XMFLOAT3	VoxelGridCenter;
	float				VoxelRadianceDataSize;				// voxel half-extent in world space units
	float				VoxelRadianceDataSizeRCP;			// 1.0 / voxel-half extent
	uint32_t			VoxelRadianceDataRes;				// voxel grid resolution
	float				VoxelRadianceDataResRCP;			// 1.0 / voxel grid resolution
	uint32_t			VoxelRadianceNumCones;
	float				VoxelRadianceNumConesRCP;
	float				VoxelRadianceMaxDistance;
	float				VoxelRadianceRayStepSize;
	uint32_t			VoxelRadianceMips;
};

struct PerObjectConstantBuffer
{
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
	DirectX::XMFLOAT4X4 WorldViewProjection;
	DirectX::XMFLOAT4X4 PrevWorldViewProjection;
};

struct ShadowConstantBuffer
{
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
};

struct PerFrameConstantBuffer
{
	DirectX::XMFLOAT4X4 ViewProjectionTex; //Required to get apply SSAO Position
	VoxelParams			VoxelData;
	DirectX::XMFLOAT4X4 ShadowView;
	DirectX::XMFLOAT4X4 ShadowProjection;
};

struct DirectionalLight
{
	DirectX::XMFLOAT3	Direction;
	float				Padding;
	DirectX::XMFLOAT3	Color;
	float				Intensity;
	
};

struct PointLight
{
	DirectX::XMFLOAT3	Position;
	float				Intensity;
	DirectX::XMFLOAT3	Color;
	float				Range;
};

struct SpotLight
{
	DirectX::XMFLOAT3	Color;
	float				Intensity;
	DirectX::XMFLOAT3	Position;
	float				Range;
	DirectX::XMFLOAT3	Direction;
	float				SpotFalloff;
};

struct LightBuffer
{
	DirectionalLight	DirLights[CMaxDirLights];
	PointLight			PointLights[CMaxPointLights];
	SpotLight			SpotLights[CMaxSpotLights];
	DirectX::XMFLOAT3	CameraPosition;
	float				NearZ;
	uint32_t			DirLightCount;
	uint32_t			PointLightCount;
	uint32_t			SpotLightCount;
	float				FarZ;
};

struct PerArmatureConstantBuffer
{
	DirectX::XMFLOAT4X4 Bones[CMaxBones];
};