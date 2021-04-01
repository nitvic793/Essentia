#include "BaseBindings.h"
#include "EngineBindings.h"
#include "Vectors.h"
#include <DirectXMath.h>
#include "EntityBase.h"
#include <memory>

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

namespace es::bindings
{
	void RegisterBindings()
	{
		auto& binding = es::Binding::GetInstance();
		binding.BindForeignClass("math.vector", "Vec3", { AllocateFloat3 , FinalizeType<XMFLOAT3> });
		binding.BindForeignClass("engine", "Entity", { AllocateEntity , FinalizeType<EntityHandle> });

		binding.BindMethod("math.vector", "Vec3", "normalize()", false, WrenFloat3Normalize);
		binding.BindMethod("math.vector", "Vec3", "length()", false, WrenFloat3Length);
		binding.BindMethod("math.vector", "Vec3", "dot(_)", false, WrenFloat3Dot);
		binding.BindMethod("math.vector", "Vec3", "cross(_)", false, WrenFloat3Cross);

		binding.BindMethod("math", "Math", "cos(_)", true, WrenCos);
		binding.BindMethod("math", "Math", "sin(_)", true, WrenSin);

		binding.BindMethod("engine", "Entity", "getPosition()", false, WrenEntityGetPosition);
		binding.BindMethod("engine", "Entity", "setPosition(_,_,_)", false, WrenEntitySetPosition);
		binding.BindMethod("engine", "Entity", "position", false, WrenEntityGetPosition);
		binding.BindMethod("engine", "Entity", "position=(_)", false, WrenEntitySetPositionVec3);

		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, x, Double);
		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, y, Double);
		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, z, Double);
	}
}
