#include "BaseBindings.h"
#include "EngineBindings.h"
#include "Vectors.h"
#include <DirectXMath.h>
#include "EntityBase.h"
#include <memory>

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

		binding.BindMethod("engine", "Entity", "getPosition()", false, WrenEntityGetPosition);

		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, x, Double);
		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, y, Double);
		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, z, Double);
	}
}
