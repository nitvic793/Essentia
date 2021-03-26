#pragma once

#include <wren.hpp>

namespace es::bindings
{
	void WrenFloat3Normalize(WrenVM* vm);
	void WrenFloat3Dot(WrenVM* vm);
	void WrenFloat3Cross(WrenVM* vm);
	void WrenFloat3Length(WrenVM* vm);
}