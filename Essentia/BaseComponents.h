#pragma once
#include "EntityBase.h"
#include "Mesh.h"
#include "Material.h"
#include "Declarations.h"
#include "Engine.h"
#include "ConstantBuffer.h"
#include "CMath.h"

/*
Serialization for predefined types
TODO: Figure out cerealization of DrawableComponent and DrawableModelComponent
*/

template<class Archive>
void serialize(Archive& archive, DirectX::XMFLOAT3& vector)
{
	archive(vector.x, vector.y, vector.z);
}

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

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(X),
			CEREAL_NVP(Y),
			CEREAL_NVP(Z)
		);
	};
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

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(X),
			CEREAL_NVP(Y),
			CEREAL_NVP(Z)
		);
	};
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

	template<class Archive> 
	void serialize(Archive& archive) 
	{
		archive(
			CEREAL_NVP(X),
			CEREAL_NVP(Y),
			CEREAL_NVP(Z)
		);
	};
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
		component.CBView = es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.Mesh = mesh;
		component.Material = material;
		return component;
	}

	template<class Archive> void serialize(Archive& a) {};
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
		component.CBView = es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.Model = model;
		return component;
	}

	template<class Archive> void serialize(Archive& a) {};
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

	template<class Archive> void serialize(Archive& archive) 
	{
		Vector3 Color(this->Color);
		Vector3 Direction(this->Direction);
		archive(
			CEREAL_NVP(Color), 
			CEREAL_NVP(Intensity),
			CEREAL_NVP(Direction)
		);
	};
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

	template<class Archive> 
	void serialize(Archive& archive) 
	{
		Vector3 Color(this->Color);
		archive(
			CEREAL_NVP(Color),
			CEREAL_NVP(Intensity),
			CEREAL_NVP(Range)
		);
	};
};

struct SkyboxComponent : public IComponent
{
	GComponent(SkyboxComponent)
	TextureID			CubeMap;
	ConstantBufferView	CBView;

	static SkyboxComponent Create(const char* skyboxFileName, TextureType type = DDS)
	{
		SkyboxComponent component;
		component.CBView = es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.CubeMap = es::CreateTexture(skyboxFileName, type, false);
		return component;
	}

	template<class Archive> void serialize(Archive& a) {};
};

// Add this component to any entity which is to be selected
struct SelectedComponent : public IComponent
{
	GComponent(SelectedComponent)

	template<class Archive> void serialize(Archive& a) {};
};

//TODO: Make camera part of Component System 
struct CameraComponent : public IComponent
{
	template<class Archive> void serialize(Archive& a) {};
};
