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