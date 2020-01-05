#pragma once

#include <DirectXMath.h>

constexpr uint32_t CMaxDirLights = 4;
constexpr uint32_t CMaxPointLights = 64;
constexpr uint32_t CMaxSpotLights = 16;

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
	float				Padding1;
	uint32_t			DirLightCount;
	uint32_t			PointLightCount;
	uint32_t			SpotLightCount;
	float				Padding2;
};