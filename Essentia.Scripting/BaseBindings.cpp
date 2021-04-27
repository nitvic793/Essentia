#include "pch.h"

#include "BaseBindings.h"
#include "EngineBindings.h"
#include "Vectors.h"
#include <DirectXMath.h>
#include "EntityBase.h"
#include <memory>

#include "Entity.h"
#include "EngineContext.h"
#include "Rotatable.h"

#pragma warning( disable : 4244 ) // Disable double to float cast warnings as they're generated through macro. *TODO*: Fix Me

using namespace DirectX;

void AllocateFloat3(WrenVM* vm)
{
	auto t = es::bindings::AllocateType<XMFLOAT3>(vm);
	*t = XMFLOAT3(
		(float)wrenGetSlotDouble(vm, 1),
		(float)wrenGetSlotDouble(vm, 2),
		(float)wrenGetSlotDouble(vm, 3)
	);
}

void AllocateFloat4(WrenVM* vm)
{
	auto t = es::bindings::AllocateType<XMFLOAT4>(vm);
	*t = XMFLOAT4(
		(float)wrenGetSlotDouble(vm, 1),
		(float)wrenGetSlotDouble(vm, 2),
		(float)wrenGetSlotDouble(vm, 3),
		(float)wrenGetSlotDouble(vm, 4)
	);
}

void WrenCos(WrenVM* vm)
{
	auto val = wrenGetSlotDouble(vm, 1);
	val = std::cos(val);
	wrenSetSlotDouble(vm, 0, val);
}

void WrenSin(WrenVM* vm)
{
	auto val = wrenGetSlotDouble(vm, 1);
	val = std::sin(val);
	wrenSetSlotDouble(vm, 0, val);
}

template<typename ComponentT>
void AllocateComponent(WrenVM* vm)
{
	ComponentT** outComp = es::bindings::AllocateTypePtr<ComponentT>(vm);
	EntityHandle* entity = (EntityHandle*)wrenGetSlotForeign(vm, 1);
	ComponentT* inComp = GContext->EntityManager->GetComponent<ComponentT>(*entity);
	*outComp = inComp;
}


namespace es::bindings
{
	void RegisterBindings()
	{
		auto& binding = es::ScriptBinding::GetInstance();
		binding.BindForeignClass("math.vector", "Vec3", { AllocateFloat3 , FinalizeType<XMFLOAT3> });
		binding.BindForeignClass("math.vector", "Vec4", { AllocateFloat3 , FinalizeType<XMFLOAT4> });
		binding.BindForeignClass("engine", "Entity", { AllocateEntity , FinalizeType<EntityHandle> });
		binding.BindForeignClass("components", "Rotatable", { AllocateComponent<Rotatable>, FinalizeType<Rotatable> });

		binding.BindMethod("math.vector", "Vec3", "normalize()", false, WrenFloat3Normalize);
		binding.BindMethod("math.vector", "Vec3", "length()", false, WrenFloat3Length);
		binding.BindMethod("math.vector", "Vec3", "dot(_)", false, WrenFloat3Dot);
		binding.BindMethod("math.vector", "Vec3", "cross(_)", false, WrenFloat3Cross);
		binding.BindMethod("math.vector", "Vec3", "+(_)", false, WrenFloat3Add);
		binding.BindMethod("math.vector", "Vec3", "-(_)", false, WrenFloat3Subtract);
		binding.BindMethod("math.vector", "Vec3", "set(_)", false, WrenFloat3Equals);

		binding.BindMethod("math", "Math", "cos(_)", true, WrenCos);
		binding.BindMethod("math", "Math", "sin(_)", true, WrenSin);

		binding.BindMethod("engine", "Entity", "getPosition()", false, WrenEntityGetPosition);
		binding.BindMethod("engine", "Entity", "setPosition(_,_,_)", false, WrenEntitySetPosition);
		binding.BindMethod("engine", "Entity", "position", false, WrenEntityGetPosition);
		binding.BindMethod("engine", "Entity", "position=(_)", false, WrenEntitySetPositionVec3);
		binding.BindMethod("engine", "Entity", "rotate(_,_)", false, WrenEntityRotate);

		binding.BindMethod("components", "Rotatable", "Speed=(_)", false, [](WrenVM* vm)
			{
				Rotatable** comp = (Rotatable**)wrenGetSlotForeign(vm, 0);
				float rhs = (float)wrenGetSlotDouble(vm, 1);
				(*comp)->Speed = rhs;
			});

		binding.BindMethod("components", "Rotatable", "Rotation=(_)", false, [](WrenVM* vm)
			{
				Rotatable** comp = (Rotatable**)wrenGetSlotForeign(vm, 0);
				float rhs = (float)wrenGetSlotDouble(vm, 1);
				(*comp)->Rotation = rhs;
			});

		binding.BindMethod("components", "Rotatable", "Rotation", false, [](WrenVM* vm)
			{
				Rotatable** comp = (Rotatable**)wrenGetSlotForeign(vm, 0);
				wrenSetSlotDouble(vm, 0, (float)(*comp)->Rotation);
			});

		binding.BindMethod("components", "Rotatable", "Speed", false, [](WrenVM* vm)
			{
				Rotatable** comp = (Rotatable**)wrenGetSlotForeign(vm, 0);
				wrenSetSlotDouble(vm, 0, (double)(*comp)->Speed);
			});

		binding.BindMethod("components", "Rotatable", "[_]", true, [](WrenVM* vm)
			{
				uint32 index = (uint32)wrenGetSlotDouble(vm, 1);
				uint32 count;
				Rotatable* components = GContext->EntityManager->GetComponents<Rotatable>(count);
				Rotatable** bytes = (Rotatable**)wrenSetSlotNewForeign(vm, 0, 0, sizeof(Rotatable**));
				(*bytes) = &components[index];
			});

		binding.BindMethod("components", "Rotatable", "count()", true, [](WrenVM* vm)
			{
				uint32 count;
				Rotatable* components = GContext->EntityManager->GetComponents<Rotatable>(count);
				wrenSetSlotDouble(vm, 0, (double)count);
			});

		binding.BindMethod("components", "Rotatable", "getEntity(_)", true, [](WrenVM* vm)
			{
				uint32 index = (uint32)wrenGetSlotDouble(vm, 1);
				uint32 count;
				auto entities = GContext->EntityManager->GetEntities<Rotatable>(count);
				wrenGetVariable(vm, "engine", "Entity", 1);
				EntityHandle* newHandle = (EntityHandle*)wrenSetSlotNewForeign(vm, 0, 1, sizeof(EntityHandle));
				*newHandle = entities[index];
			});

		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, x, Double);
		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, y, Double);
		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, z, Double);

		MWrenBindGetterSetter(math.vector, Vec4, XMFLOAT4, Float4, x, Double);
		MWrenBindGetterSetter(math.vector, Vec4, XMFLOAT4, Float4, y, Double);
		MWrenBindGetterSetter(math.vector, Vec4, XMFLOAT4, Float4, z, Double);
		MWrenBindGetterSetter(math.vector, Vec4, XMFLOAT4, Float4, w, Double);
	}
}
