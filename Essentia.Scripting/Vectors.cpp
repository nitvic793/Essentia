#include "Vectors.h"
#include <DirectXMath.h>
#include <memory>

using namespace DirectX;

namespace es::bindings
{
	void WrenFloat3Normalize(WrenVM* vm)
	{
		XMFLOAT3* vector = (XMFLOAT3*)wrenGetSlotForeign(vm, 0);
		XMStoreFloat3(vector, XMVector3Normalize(XMLoadFloat3(vector)));
	}

	void WrenFloat3Dot(WrenVM* vm)
	{
		XMFLOAT3* lhs = (XMFLOAT3*)wrenGetSlotForeign(vm, 0);
		XMFLOAT3* rhs = (XMFLOAT3*)wrenGetSlotForeign(vm, 1);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Dot(XMLoadFloat3(lhs), XMLoadFloat3(rhs)));
		wrenSetSlotDouble(vm, 0, result.x);
	}

	void WrenFloat3Cross(WrenVM* vm)
	{
		XMFLOAT3* lhs = (XMFLOAT3*)wrenGetSlotForeign(vm, 0);
		XMFLOAT3* rhs = (XMFLOAT3*)wrenGetSlotForeign(vm, 1);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Cross(XMLoadFloat3(lhs), XMLoadFloat3(rhs)));
		void* bytes = wrenSetSlotNewForeign(vm, 0, 0, sizeof(XMFLOAT3));
		memcpy(bytes, &result, sizeof(XMFLOAT3));
	}

	void WrenFloat3Length(WrenVM* vm)
	{
		XMFLOAT3* vector = (XMFLOAT3*)wrenGetSlotForeign(vm, 0);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Length(XMLoadFloat3(vector)));
		wrenSetSlotDouble(vm, 0, result.x);
	}
}