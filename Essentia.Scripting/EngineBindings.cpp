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
		//EntityHandle* entity = (EntityHandle*)wrenGetSlotForeign(vm, 0);
		//auto transform = GContext->EntityManager->GetTransform(*entity);
		//auto handle = ClassHandleMap::GetHandle("math.vector", "Vec3", vm);
		//void* bytes = wrenSetSlotNewForeign(vm, 0, 0, sizeof(XMFLOAT3)); 
		//memcpy(bytes, transform.Position, sizeof(XMFLOAT3));
		//wrenSetSlotHandle(vm, 0, handle);
	}
}