#include "BaseBindings.h"
#include <DirectXMath.h>
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
		es::Binding::GetInstance().BindForeignClass("math.vector", "Vec3", { AllocateFloat3 , FinalizeType<XMFLOAT3> });

		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, x, Double);
		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, y, Double);
		MWrenBindGetterSetter(math.vector, Vec3, XMFLOAT3, Float3, z, Double);
	}
}
