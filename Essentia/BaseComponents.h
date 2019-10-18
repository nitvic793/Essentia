#pragma once
#include "EntityBase.h"
#include "Mesh.h"
#include "Material.h"
#include "Declarations.h"
#include "Engine.h"
#include "ConstantBuffer.h"

struct PositionComponent : public IComponent
{
	GComponent(PositionComponent)
	float X;
	float Y;
	float Z;
	
	const PositionComponent& operator=(const DirectX::XMFLOAT3& position)
	{
		this->X = position.x;
		this->Y = position.y;
		this->Z = position.z;
		return *this;
	}

	operator DirectX::XMFLOAT3()
	{
		return DirectX::XMFLOAT3(X, Y, Z);
	}
};

struct RotationComponent : public IComponent
{
	GComponent(RotationComponent)
	float X;
	float Y;
	float Z;

	const RotationComponent& operator=(const DirectX::XMFLOAT3& position)
	{
		this->X = position.x;
		this->Y = position.y;
		this->Z = position.z;
		return *this;
	}

	operator DirectX::XMFLOAT3()
	{
		return DirectX::XMFLOAT3(X, Y, Z);
	}
};

struct ScaleComponent : public IComponent
{
	GComponent(ScaleComponent)
	float X;
	float Y;
	float Z;

	const ScaleComponent& operator=(const DirectX::XMFLOAT3& position)
	{
		this->X = position.x;
		this->Y = position.y;
		this->Z = position.z;
		return *this;
	}

	operator DirectX::XMFLOAT3()
	{
		return DirectX::XMFLOAT3(X, Y, Z);
	}
};

struct IDrawable : public IComponent {};

struct DrawableComponent : public IDrawable
{
	GComponent(DrawableComponent)
	MeshHandle			Mesh;
	MaterialHandle		Material;
	ConstantBufferView	CBView;
	DirectX::XMFLOAT4X4 WorldViewProjection;
	DirectX::XMFLOAT4X4 PrevWorldViewProjection;

	static DrawableComponent Create(MeshHandle mesh, MaterialHandle material)
	{
		DrawableComponent component;
		component.CBView = Es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.Mesh = mesh;
		component.Material = material;
		return component;
	}
};

// To support models with inbuilt textures/materials
struct DrawableModelComponent : public IDrawable
{
	GComponent(DrawableModelComponent)
	ModelHandle			Model;
	ConstantBufferView	CBView;
	DirectX::XMFLOAT4X4 WorldViewProjection;
	DirectX::XMFLOAT4X4 PrevWorldViewProjection;

	static DrawableModelComponent Create(ModelHandle model)
	{
		DrawableModelComponent component;
		component.CBView = Es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.Model = model;
		return component;
	}
};

struct ILight : public IComponent {};

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
		return { Direction, 0, Color, Intensity };
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

struct SkyboxComponent : public IComponent
{
	GComponent(SkyboxComponent)
	TextureID			CubeMap;
	ConstantBufferView	CBView;

	static SkyboxComponent Create(const char* skyboxFileName, TextureType type = DDS)
	{
		SkyboxComponent component;
		component.CBView = Es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.CubeMap = Es::CreateTexture(skyboxFileName, type, false);
		return component;
	}
};

// Add this component to any entity which is to be selected
struct SelectedComponent : public IComponent
{
	GComponent(SelectedComponent)
};

//TODO: Make camera part of Component System 
struct CameraComponent : public IComponent
{
};
