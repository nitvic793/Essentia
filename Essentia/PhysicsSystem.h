#pragma once

#include "Entity.h"
#include "System.h"
#include "Engine.h"
#include "PhysicsContext.h"
#include "Memory.h"

class PhysicsSystem : public ISystem
{
public:
	virtual void Initialize() override;
	virtual void Update(float deltaTime, float totalTime) override;
	virtual void Destroy() override;
private:
	ScopedPtr<PhysicsContext> physicsContext;

	float accumulator = 0.f;
	static constexpr float CPhysicsStepSize = 1.f / 60.f;
};