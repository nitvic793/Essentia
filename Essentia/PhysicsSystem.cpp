#include "PhysicsSystem.h"
#include "PxPhysicsAPI.h"
#include "GameStateManager.h"
#include "PhysicsComponents.h"

using namespace physx;
using namespace DirectX;
/**
 * Instantiate PhysX rigid body for given entity if a Collider is present.
 *
 * \param entity
 */
static void ConsolidateRigidBody(EntityHandle entity)
{
	auto physics = GPhysicsContext->GetPhysicsFactory();
	auto scene = GPhysicsContext->GetPhysicsScene();
	auto rigidBodyComponent = GContext->EntityManager->GetComponent<RigidBodyComponent>(entity);

	if (!rigidBodyComponent->RigidBody)
	{
		auto sphereCollider = GContext->EntityManager->GetComponentManager()->TryGetComponent<SphereCollider>(entity);
		auto boxCollider = GContext->EntityManager->GetComponentManager()->TryGetComponent<BoxCollider>(entity);
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
		auto transformRef = GContext->EntityManager->GetTransform(entity);
		physx::PxTransform transform;// (physx::PxMat44(&worldMatrix.m[0][0]));
		transform.p = PxVec3(transformRef.Position->x, transformRef.Position->y, transformRef.Position->z);
		transform.q = PxQuat(transformRef.Rotation->x, transformRef.Rotation->y, transformRef.Rotation->z, transformRef.Rotation->w);
		bool sane = transform.q.isSane();
		bool finite = transform.p.isFinite();
		rigidBodyComponent->RigidBody = physics->createRigidDynamic(transform);
		rigidBodyComponent->RigidBody->attachShape(*shape);
		PxRigidBodyExt::updateMassAndInertia(*rigidBodyComponent->RigidBody, rigidBodyComponent->Mass);
		scene->addActor(*rigidBodyComponent->RigidBody);

		shape->release();
	}
}

static void ConsolidateRigidBodyStatic(EntityHandle entity)
{
	auto physics = GPhysicsContext->GetPhysicsFactory();
	auto scene = GPhysicsContext->GetPhysicsScene();
	auto comp = GContext->EntityManager->GetComponent<RigidBodyStaticComponent>(entity);

	if (!comp->RigidBodyStatic)
	{
		auto plane = GContext->EntityManager->GetComponentManager()->TryGetComponent<PlaneCollider>(entity);
		auto sphereCollider = GContext->EntityManager->GetComponentManager()->TryGetComponent<SphereCollider>(entity);
		auto boxCollider = GContext->EntityManager->GetComponentManager()->TryGetComponent<BoxCollider>(entity);
		physx::PxShape* shape = nullptr;

		if (plane)
		{
			comp->RigidBodyStatic = PxCreatePlane(
				*physics,
				PxPlane(plane->NormalX, plane->NormalY, plane->NormalZ, plane->Distance),
				*GPhysicsContext->GetDefaultMaterial()
			);
		}
		else if (sphereCollider)
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

		if (shape)
		{
			DirectX::XMFLOAT4X4 worldMatrix = GContext->EntityManager->GetWorldMatrix(entity);
			auto transformRef = GContext->EntityManager->GetTransform(entity);
			physx::PxTransform transform;// (physx::PxMat44(&worldMatrix.m[0][0]));
			transform.p = PxVec3(transformRef.Position->x, transformRef.Position->y, transformRef.Position->z);
			transform.q = PxQuat(transformRef.Rotation->x, transformRef.Rotation->y, transformRef.Rotation->z, transformRef.Rotation->w);
			bool sane = transform.q.isSane();
			bool finite = transform.p.isFinite();
			comp->RigidBodyStatic = PxCreateStatic(*physics, transform, *shape);
		}

		DirectX::XMFLOAT4X4 worldMatrix = GContext->EntityManager->GetWorldMatrix(entity);
		physx::PxTransform transform(physx::PxMat44(&worldMatrix.m[0][0]));
		scene->addActor(*comp->RigidBodyStatic);
	}
}

/**
 * Update transform of entities from PhysX rigid body.
 *
 * \param entity Entity to update
 */
static void UpdateEntityTransform(EntityHandle entity)
{
	auto physics = GPhysicsContext->GetPhysicsFactory();
	auto scene = GPhysicsContext->GetPhysicsScene();

	auto comp = GContext->EntityManager->GetComponent<RigidBodyComponent>(entity);

	PxTransform transform = comp->RigidBody->getGlobalPose();

	auto esTransform = GContext->EntityManager->GetTransform(entity);
	*esTransform.Position = DirectX::XMFLOAT3(&transform.p.x);
	*esTransform.Rotation = DirectX::XMFLOAT4(&transform.q.x);
}


void PhysicsSystem::Initialize()
{
	physicsContext = MakeScoped<PhysicsContext>();
	physicsContext->Initialize();
	GPhysicsContext = physicsContext.Get();

	es::GEventBus->Subscribe(this, &PhysicsSystem::OnTransformUpdate);
	es::GEventBus->Subscribe(this, &PhysicsSystem::OnComponentUpdate);
	es::GEventBus->Subscribe(this, &PhysicsSystem::OnSphereColliderAdd);
	es::GEventBus->Subscribe(this, &PhysicsSystem::OnBoxColliderAdd);
}

void PhysicsSystem::Update(float deltaTime, float totalTime)
{
	uint32 count = 0;
	auto entities = GContext->EntityManager->GetEntities<RigidBodyStaticComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		ConsolidateRigidBodyStatic(entities[i]);
	}

	entities = GContext->EntityManager->GetEntities<RigidBodyComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		ConsolidateRigidBody(entities[i]);
	}

	if (GContext->GameStateManager->IsPlaying())
	{
		accumulator += deltaTime;
		if (accumulator < CPhysicsStepSize)
			return;

		accumulator -= CPhysicsStepSize;
		physicsContext->mScene->simulate(CPhysicsStepSize);

		physicsContext->mScene->fetchResults(true);
		for (uint32 i = 0; i < count; ++i)
		{
			UpdateEntityTransform(entities[i]);
		}
	}
}

void PhysicsSystem::Destroy()
{
	physicsContext->Destroy();
}

void PhysicsSystem::OnTransformUpdate(TransformUpdateEvent* event)
{
	auto comp = GContext->EntityManager->GetComponentManager()->TryGetComponent<RigidBodyComponent>(event->entity);

	if (comp != nullptr)
	{
		auto esTransform = GContext->EntityManager->GetTransform(event->entity);
		auto p = esTransform.Position;
		auto q = esTransform.Rotation;
		PxTransform transform(PxVec3(p->x, p->y, p->z), PxQuat(q->x, q->y, q->z, q->w));
		comp->RigidBody->setGlobalPose(transform);
	}
}

void PhysicsSystem::OnComponentUpdate(IComponentUpdateEvent* event)
{
	if (event->componentData.ComponentName != "PositionComponent" && event->componentData.ComponentName != "RotationComponent")
	{
		return;
	}

	auto comp = GContext->EntityManager->GetComponentManager()->TryGetComponent<RigidBodyComponent>(event->entity);
	if (comp != nullptr)
	{
		auto esTransform = GContext->EntityManager->GetTransform(event->entity);
		auto p = esTransform.Position;
		auto q = esTransform.Rotation;
		PxTransform transform(PxVec3(p->x, p->y, p->z), PxQuat(q->x, q->y, q->z, q->w));
		comp->RigidBody->setGlobalPose(transform);
	}
}

void PhysicsSystem::OnSphereColliderAdd(ComponentAddEvent<SphereCollider>* event)
{
	auto drawable = GContext->EntityManager->GetComponent<DrawableComponent>(event->entity);
	auto world = GContext->EntityManager->GetWorldMatrix(event->entity);
	auto box = GContext->MeshManager->GetBoundingBox(drawable->Mesh);
	box.Transform(box, XMLoadFloat4x4(&world));
	event->component->Radius = box.Extents.x;
}

void PhysicsSystem::OnBoxColliderAdd(ComponentAddEvent<BoxCollider>* event)
{
	auto drawable = GContext->EntityManager->GetComponent<DrawableComponent>(event->entity);
	auto world = GContext->EntityManager->GetWorldMatrix(event->entity);
	auto box = GContext->MeshManager->GetBoundingBox(drawable->Mesh);
	box.Transform(box, XMLoadFloat4x4(&world));
	event->component->HalfExtentX = box.Extents.x;
	event->component->HalfExtentY = box.Extents.y;
	event->component->HalfExtentZ = box.Extents.z;
}
