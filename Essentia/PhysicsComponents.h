#pragma once

#include "Declarations.h"
#include "EntityBase.h"
#include "PhysicsContext.h"
#include "Entity.h"

enum RigidBodyType
{
	kRigidBodySphere,
	kRigidBodyBox
};

struct SphereCollider : public IComponent
{
	float Radius = 1.f;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(Radius)
		);
	}

	GComponent(SphereCollider);
};

struct BoxCollider : public IComponent
{
	float HalfExtentX = 0.5f;
	float HalfExtentY = 0.5f;
	float HalfExtentZ = 0.5f;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(HalfExtentX),
			CEREAL_NVP(HalfExtentY),
			CEREAL_NVP(HalfExtentZ)
		);
	}

	GComponent(BoxCollider);
};

struct PlaneCollider : public IComponent
{
	float NormalX = 0.f;
	float NormalY = 1.f;
	float NormalZ = 0.f;
	float Distance = 10.f;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(NormalX),
			CEREAL_NVP(NormalY),
			CEREAL_NVP(NormalZ),
			CEREAL_NVP(Distance)
		);
	}

	GComponent(PlaneCollider);
};


struct RigidBodyComponent : public IComponent
{
	float Mass = 10.f;
	physx::PxRigidDynamic* RigidBody = nullptr;

	template<class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(Mass)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(Mass)
		);
	};

	GComponent(RigidBodyComponent);
};

struct RigidBodyStaticComponent : public IComponent
{
	physx::PxRigidStatic* RigidBodyStatic = nullptr;

	template<class Archive>
	void save(Archive& archive) const
	{
	};

	template<class Archive>
	void load(Archive& archive)
	{
	};

	GComponent(RigidBodyStaticComponent);
};