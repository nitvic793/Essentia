#pragma once

#include <wren.hpp>

namespace es::bindings
{
	void WrenFloat3Normalize(WrenVM* vm);
	void WrenFloat3Dot(WrenVM* vm);
	void WrenFloat3Cross(WrenVM* vm);
	void WrenFloat3Length(WrenVM* vm);
	void WrenFloat3Add(WrenVM* vm);
	void WrenFloat3Subtract(WrenVM* vm);
	void WrenFloat3Equals(WrenVM* vm);
}