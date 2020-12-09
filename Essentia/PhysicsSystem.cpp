#include "PhysicsSystem.h"
#include "PxPhysicsAPI.h"
#include "GameStateManager.h"
#include "PhysicsComponents.h"

using namespace physx;

static void ConsolidateRigidBody(EntityHandle entity)
{
	auto physics = GPhysicsContext->GetPhysicsFactory();
	auto scene = GPhysicsContext->GetPhysicsScene();

	auto rigidBodyComponent = GContext->EntityManager->GetComponent<RigidBodyComponent>(entity);

	if (!rigidBodyComponent->RigidBody)
	{
		auto sphereCollider = GContext->EntityManager->GetComponent<SphereCollider>(entity);
		auto boxCollider = GContext->EntityManager->GetComponent<BoxCollider>(entity);
		physx::PxShape* shape = nullptr;

		if (sphereCollider)
		{
			auto sphere = physx::PxSphereGeometry(sphereCollider->Radius);
			shape = physics->createShape(sphere, *GPhysicsContext->GetDefaultMaterial());
		}
		else if (boxCollider)
		{
			auto box = physx::PxBoxGeometry(boxCollider->HalfExtentX, boxCollider->HalfExtentY, boxCollider->HalfExtentZ);
			shape = physics->createShape(box, *GPhysicsContext->GetDefaultMaterial());
		}
		else
		{
			return; // Don't instantiate Rigid Body if collider is not present
		}

		DirectX::XMFLOAT4X4 worldMatrix = GContext->EntityManager->GetWorldMatrix(entity);
		physx::PxTransform transform(physx::PxMat44(&worldMatrix.m[0][0]));

		rigidBodyComponent->RigidBody = physics->createRigidDynamic(transform);
		rigidBodyComponent->RigidBody->attachShape(*shape);
		PxRigidBodyExt::updateMassAndInertia(*rigidBodyComponent->RigidBody, rigidBodyComponent->Mass);
		scene->addActor(*rigidBodyComponent->RigidBody);

		shape->release();
	}
}


void PhysicsSystem::Initialize()
{
	physicsContext = MakeScoped<PhysicsContext>();
	physicsContext->Initialize();
}

void PhysicsSystem::Update(float deltaTime, float totalTime)
{
	if (GContext->GameStateManager->IsPlaying())
	{
		accumulator += deltaTime;
		if (accumulator < CPhysicsStepSize)
			return;

		accumulator -= CPhysicsStepSize;
		physicsContext->mScene->simulate(CPhysicsStepSize);

		physicsContext->mScene->fetchResults(true);
	}

	uint32 count = 0;
	auto entities = GContext->EntityManager->GetEntities<RigidBodyComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		ConsolidateRigidBody(entities[i]);
	}
} 

void PhysicsSystem::Destroy()
{
	physicsContext->Destroy();
}
