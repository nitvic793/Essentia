#pragma once

#include "Declarations.h"
#include "EntityBase.h"
#include "PhysicsContext.h"
#include "Entity.h"

enum RigidBodyType
{
	kRigidBodySphere,
	kRIgidBodyBox
};

struct RigidBodyComponent : public IComponent
{
	float Radius = 1.f;
	RigidBodyType RigidBodyType = kRigidBodySphere;
	physx::PxRigidDynamic* RigidBody = nullptr;

	static RigidBodyComponent Create(float radius, EntityHandle handle)
	{
		RigidBodyComponent component = {};
		auto physics = GPhysicsContext->GetPhysicsFactory();
		auto scene = GPhysicsContext->GetPhysicsScene();
		auto sphere = physx::PxSphereGeometry(radius);
		physx::PxShape* shape = physics->createShape(sphere, *GPhysicsContext->GetDefaultMaterial());
		DirectX::XMFLOAT4X4 worldMatrix = GContext->EntityManager->GetWorldMatrix(handle);

		physx::PxTransform transform(physx::PxMat44(&worldMatrix.m[0][0]));

		component.RigidBody = physics->createRigidDynamic(transform);
		component.RigidBody->attachShape(*shape);
		physx::PxRigidBodyExt::updateMassAndInertia(*component.RigidBody, 10.0f);
		scene->addActor(*component.RigidBody);

		component.Radius = radius;
		
		shape->release();
	}

	template<class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(Radius),
			CEREAL_NVP(RigidBodyType)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(Radius),
			CEREAL_NVP(RigidBodyType)
		);
	};

	GComponent(RigidBodySphereComponent);
};