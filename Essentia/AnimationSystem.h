#pragma once


#include "Entity.h"
#include "System.h"
#include "Engine.h"
#include "Renderer.h"
#include "AnimationComponent.h"

using namespace DirectX;


class AnimationSystem : public ISystem
{
public:
	virtual void Initialize();
	virtual void Update(float deltaTime, float totalTime);

private:
	void BoneTransform(AnimationComponent& animComponent, const Animation& animation);
	void ReadNodeHeirarchy(AnimationComponent& animComponent, const Animation& animation, float animationTime);
};