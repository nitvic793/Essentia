#pragma once
#include "EntityBase.h"
#include "Mesh.h"
#include "Material.h"
#include "Declarations.h"
#include "Engine.h"
#include "ConstantBuffer.h"
#include "CMath.h"
#include "Camera.h"

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

struct BoundingOrientedBoxComponent : public IComponent
{
	DirectX::BoundingOrientedBox BoundingOrientedBox;
	template<class Archive>
	void serialize(Archive& archive){}
	GComponent(BoundingOrientedBoxComponent)
};


struct CameraComponent : public IComponent
{
	Camera CameraInstance;

	CameraComponent() :
		CameraInstance(1920, 1080) {};

	CameraComponent(int width, int height) : 
		CameraInstance((float)width, (float)height)
	{}

	template<class Archive> 
	void serialize(Archive& archive) 
	{
		archive(
			CEREAL_NVP(CameraInstance.Width),
			CEREAL_NVP(CameraInstance.Height),
			CEREAL_NVP(CameraInstance.NearZ),
			CEREAL_NVP(CameraInstance.FarZ),
			CEREAL_NVP(CameraInstance.FieldOfView),
			CEREAL_NVP(CameraInstance.IsOrthographic)
		);
	}

	static CameraComponent Create(int width = 1920, int height = 1080)
	{
		CameraComponent component(width, height);
		return component;
	}

	GComponent(CameraComponent)
};
