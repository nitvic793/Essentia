#pragma once

#include <wren.hpp>

namespace es::bindings
{
	void AllocateEntity(WrenVM* vm);
	void WrenEntityGetPosition(WrenVM* vm);
}