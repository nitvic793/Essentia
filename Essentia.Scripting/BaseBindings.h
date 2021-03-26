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

	class ClassHandleMap
	{
	public:
		static ClassHandleMap& GetInstance() 
		{
			static ClassHandleMap instance;
			return instance;
		}

		static WrenHandle* GetHandle(std::string_view module, std::string_view className, WrenVM* vm)
		{
			auto& instance = GetInstance();
			std::string name{ module };
			name += className;
			if (!instance.Exists(name))
				Register(module, className, vm);

			return instance.handleMap[name];
		}

		static void Register(std::string_view module, std::string_view className, WrenVM* vm)
		{
			std::string name{ module };
			name += className;

			wrenGetVariable(vm, module.data(), className.data(), 0);
			wrenEnsureSlots(vm, 1);
			auto handle = wrenGetSlotHandle(vm, 0);
			GetInstance().AddHandle(name, handle);
		}

		void AddHandle(std::string& name, WrenHandle* handle)
		{
			handleMap[name] = handle;
		}

		bool Exists(std::string& name) const 
		{
			return handleMap.find(name) != handleMap.end();
		}

	protected:
		std::unordered_map<std::string, WrenHandle*> handleMap;
	};
}
