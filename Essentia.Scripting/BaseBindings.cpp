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

MWrenGetterDouble(DirectX::XMFLOAT3, Float3, x);
MWrenGetterDouble(DirectX::XMFLOAT3, Float3, y);
MWrenGetterDouble(DirectX::XMFLOAT3, Float3, z);

MWrenSetterDouble(DirectX::XMFLOAT3, Float3, x);
MWrenSetterDouble(DirectX::XMFLOAT3, Float3, y);
MWrenSetterDouble(DirectX::XMFLOAT3, Float3, z);

namespace es::bindings
{
	void RegisterBindings()
	{
		es::Binding::GetInstance().BindForeignClass("math.vector", "Vec3", { AllocateFloat3 , FinalizeType<XMFLOAT3> });
		MWrenRegisterSetter(math.vector, MWrenGetSetterName(DirectX::XMFLOAT3, Float3, x), Vec3, x);
		MWrenRegisterGetter(math.vector, MWrenGetGetterName(DirectX::XMFLOAT3, Float3, x), Vec3, x);

		MWrenRegisterSetter(math.vector, MWrenGetSetterName(DirectX::XMFLOAT3, Float3, y), Vec3, y);
		MWrenRegisterGetter(math.vector, MWrenGetGetterName(DirectX::XMFLOAT3, Float3, y), Vec3, y);

		MWrenRegisterSetter(math.vector, MWrenGetSetterName(DirectX::XMFLOAT3, Float3, z), Vec3, z);
		MWrenRegisterGetter(math.vector, MWrenGetGetterName(DirectX::XMFLOAT3, Float3, z), Vec3, z);
	}
}
