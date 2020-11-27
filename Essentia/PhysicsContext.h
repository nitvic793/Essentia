#pragma once

#include "PxPhysicsAPI.h"

// Context initialized with Physics System
class PhysicsContext
{
public:
	void Initialize();
	void Destroy();

	inline physx::PxPhysics*	GetPhysicsFactory() { return mPhysics; }
	inline physx::PxScene*		GetPhysicsScene() { return mScene; }
	inline physx::PxMaterial*	GetDefaultMaterial() { return mDefaultMaterial;  }
private:
	physx::PxDefaultAllocator		mAllocator;
	physx::PxDefaultErrorCallback	mErrorCallback;

	physx::PxFoundation*			mFoundation = NULL;
	physx::PxPhysics*				mPhysics = NULL;

	physx::PxDefaultCpuDispatcher*	mDispatcher = NULL;
	physx::PxScene*					mScene = NULL;

	physx::PxPvd*					mPvd = NULL;
	physx::PxMaterial*				mDefaultMaterial = NULL;

	friend class PhysicsSystem;
};

extern PhysicsContext* GPhysicsContext;