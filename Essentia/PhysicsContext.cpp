#include "PhysicsContext.h"

PhysicsContext* GPhysicsContext = nullptr;

using namespace physx;

#ifndef PVD_HOST
#define PVD_HOST "127.0.0.1"	//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.
#endif 

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

void PhysicsContext::Initialize()
{
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);

	mPvd = PxCreatePvd(*mFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(), true, mPvd);

	PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	mDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = mDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	mScene = mPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	mDefaultMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}

void PhysicsContext::Destroy()
{
	PX_RELEASE(mScene);
	PX_RELEASE(mDispatcher);
	PX_RELEASE(mPhysics);
	if (mPvd)
	{
		PxPvdTransport* transport = mPvd->getTransport();
		mPvd->release();	mPvd = NULL;
		PX_RELEASE(transport);
	}

	PX_RELEASE(mFoundation);
}
