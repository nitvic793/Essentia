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

struct LightComponent
{
	GComponent(LightComponent)

	DirectX::XMFLOAT3	Color;
	float				Intensity;
	DirectX::XMFLOAT3	Direction;
	LightType			LightType;
	float				Range;

	static LightComponent CreateDirectional(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1, 1, 1), float intensity = 1.f)
	{
		LightComponent light;
		light.LightType = LightType::Directional;
		light.Color = color;
		light.Direction = direction;
		light.Intensity = intensity;
		return light;
	}

	static LightComponent CreatePoint(const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1, 1, 1), float intensity = 1.f, float range = 5.f)
	{
		LightComponent light;
		light.LightType = LightType::Point;
		light.Color = color;
		light.Range = range;
		light.Intensity = intensity;
		return light;
	}
};