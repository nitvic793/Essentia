#pragma once
#include "EntityBase.h"
#include "Mesh.h"
#include "Material.h"
#include "Declarations.h"

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
	MeshView			Mesh;
	Material			Material;
	ConstantBufferView	CBView;
};