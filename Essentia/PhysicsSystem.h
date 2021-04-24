#pragma once

#include "Entity.h"
#include "System.h"
#include "Engine.h"
#include "PhysicsContext.h"
#include "Memory.h"
#include "EventTypes.h"

struct SphereCollider;
struct BoxCollider;

class PhysicsSystem : public ISystem
{
public:
	virtual void Initialize() override;
	virtual void Update(float deltaTime, float totalTime) override;
	virtual void Destroy() override;
	virtual void Reset() override;
private:
	ScopedPtr<PhysicsContext> physicsContext;

	float accumulator = 0.f;
	static constexpr float CPhysicsStepSize = 1.f / 60.f;

	void OnTransformUpdate(TransformUpdateEvent* event);
	void OnComponentUpdate(IComponentUpdateEvent* event);
	void OnSphereColliderAdd(ComponentAddEvent<SphereCollider>* event);
	void OnBoxColliderAdd(ComponentAddEvent<BoxCollider>* event);
};