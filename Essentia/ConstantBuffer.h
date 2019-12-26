#pragma once

#include <DirectXMath.h>

struct PerObjectConstantBuffer
{
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
	DirectX::XMFLOAT4X4 WorldViewProjection;
	DirectX::XMFLOAT4X4 PrevWorldViewProjection;
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
	DirectionalLight	DirLight;
	PointLight			PointLight;
	SpotLight			SpotLight;
	DirectX::XMFLOAT3	CameraPosition;
	float				Padding;
};