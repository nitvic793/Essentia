#include "pch.h"
#include "AnimationSystem.h"

void AnimationSystem::Initialize()
{
}

void AnimationSystem::Update(float deltaTime, float totalTime)
{
	uint32 count = 0;
	AnimationComponent* animComponents = GContext->EntityManager->GetComponents<AnimationComponent>(count);

	for (uint32 i = 0; i < count; ++i) 
	{

	}
}
