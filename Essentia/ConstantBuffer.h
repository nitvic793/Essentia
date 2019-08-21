#pragma once

#include <DirectXMath.h>

struct PerObjectConstantBuffer
{
	DirectX::XMFLOAT4X4 World;
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

struct LightBuffer
{
	DirectionalLight	DirLight;
	PointLight			PointLight;
	DirectX::XMFLOAT3	CameraPosition;
};