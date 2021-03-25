#pragma once

#include <wren.hpp>
#include <DirectXMath.h>
#include "BindingHelpers.h"
#include "Binding.h"

// https://nelari.us/post/template-function-args/

namespace es::bindings
{
	template<typename T>
	T* AllocateType(WrenVM* vm) 
	{
		void* bytes = wrenSetSlotNewForeign(vm, 0, 0, sizeof(T));
		return new (bytes) T();
	}

	template<typename T>
	void FinalizeType(void* bytes) 
	{
		T* type = (T*)bytes;
		type->~T();
	}

	void RegisterBindings();
}
