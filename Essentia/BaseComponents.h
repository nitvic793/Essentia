#pragma once
#include "EntityBase.h"
#include "Mesh.h"
#include "Material.h"
#include "Declarations.h"
#include "Engine.h"
#include "ConstantBuffer.h"

struct PositionComponent
{
	GComponent(PositionComponent)
	DirectX::XMFLOAT3 Position;
};

struct RotationComponent
{
	GComponent(RotationComponent)
	DirectX::XMFLOAT3 Rotation;
};

struct ScaleComponent
{
	GComponent(ScaleComponent)
	DirectX::XMFLOAT3 Scale;
};

struct DrawableComponent
{
	GComponent(DrawableComponent)
	MeshHandle			Mesh;
	MaterialHandle		Material;
	ConstantBufferView	CBView;

	static DrawableComponent Create(MeshHandle mesh, MaterialHandle material)
	{
		DrawableComponent component;
		component.CBView = Es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.Mesh = mesh;
		component.Material = material;
		return component;
	}
};

struct ILight {};

struct DirectionalLightComponent : public ILight
{
	GComponent(DirectionalLightComponent)

	DirectX::XMFLOAT3	Color;
	float				Intensity;
	DirectX::XMFLOAT3	Direction;
	float				Padding;

	static DirectionalLightComponent Create(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1, 1, 1), float intensity = 1.f)
	{
		DirectionalLightComponent light;
		light.Color = color;
		light.Direction = direction;
		light.Intensity = intensity;
		return light;
	}

	DirectionalLight GetLight()
	{
		return { Direction, Color };
	}
};

struct PointLightComponent : public ILight
{
	GComponent(PointLightComponent)
	DirectX::XMFLOAT3	Color;
	float				Intensity;
	float				Range;
	float				Padding[3];

	static PointLightComponent Create(const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1, 1, 1), float intensity = 1.f, float range = 5.f)
	{
		PointLightComponent light;
		light.Color = color;
		light.Range = range;
		light.Intensity = intensity;
		return light;
	}

	PointLight GetLight()
	{
		return { {}, Intensity, Color, Range };
	}
};