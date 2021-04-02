#include "pch.h"

#include "Entity.h"
#include "EngineContext.h"

#include "BaseBindings.h"
#include "EngineBindings.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace es::bindings
{
	void AllocateEntity(WrenVM* vm)
	{
		EntityHandle* entity = AllocateType<EntityHandle>(vm);
		const char* entityName = wrenGetSlotString(vm, 1);
		*entity = GContext->EntityManager->GetEntityByName(entityName);
	}

	void WrenEntityGetPosition(WrenVM* vm)
	{
		EntityHandle* entity = (EntityHandle*)wrenGetSlotForeign(vm, 0);
		auto transform = GContext->EntityManager->GetTransform(*entity);
		wrenGetVariable(vm, "math.vector", "Vec3", 1);
		void* bytes = wrenSetSlotNewForeign(vm, 0, 1, sizeof(XMFLOAT3)); 
		memcpy(bytes, transform.Position, sizeof(XMFLOAT3));
	}

	void WrenEntitySetPosition(WrenVM* vm)
	{
		EntityHandle* entity = (EntityHandle*)wrenGetSlotForeign(vm, 0);
		auto transform = GContext->EntityManager->GetTransform(*entity);
		auto x = (float)wrenGetSlotDouble(vm, 1);
		auto y = (float)wrenGetSlotDouble(vm, 2);
		auto z = (float)wrenGetSlotDouble(vm, 3);
		*transform.Position = XMFLOAT3(x, y, z);
	}

	void WrenEntitySetPositionVec3(WrenVM* vm)
	{
		EntityHandle* entity = (EntityHandle*)wrenGetSlotForeign(vm, 0);
		auto transform = GContext->EntityManager->GetTransform(*entity);
		auto val = (XMFLOAT3*)wrenGetSlotForeign(vm, 1);
		memcpy(transform.Position, val, sizeof(XMFLOAT3));
	}

}