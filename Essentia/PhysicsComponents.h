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

	GComponent(RigidBodySphereComponent);
};