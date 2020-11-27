#include "PhysicsSystem.h"
#include "PxPhysicsAPI.h"
#include "GameStateManager.h"

using namespace physx;


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
} 

void PhysicsSystem::Destroy()
{
	physicsContext->Destroy();
}
