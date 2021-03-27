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

	class WrenHandleMap
	{
	public:
		static WrenHandleMap& GetInstance() 
		{
			static WrenHandleMap instance;
			return instance;
		}

		static WrenHandle* GetClassHandle(std::string_view module, std::string_view className, WrenVM* vm)
		{
			auto& instance = GetInstance();
			std::string name{ module };
			name += className;
			if (!instance.Exists(name))
				RegisterWrenClass(module, className, vm);

			return instance.classHandleMap[name];
		}

		static void RegisterWrenClass(std::string_view module, std::string_view className, WrenVM* vm)
		{
			std::string name{ module };
			name += className;
			GetInstance().vmInstance = vm;
			wrenGetVariable(vm, module.data(), className.data(), 0);
			wrenEnsureSlots(vm, 1);
			auto handle = wrenGetSlotHandle(vm, 0);
			GetInstance().AddHandle(name, handle);
		}

		void AddHandle(std::string& name, WrenHandle* handle)
		{
			classHandleMap[name] = handle;
		}

		bool Exists(std::string& name) const 
		{
			return classHandleMap.find(name) != classHandleMap.end();
		}

		void ReleaseHandles()
		{
			if (!vmInstance) return;
			for (auto& handle : classHandleMap)
			{
				wrenReleaseHandle(vmInstance, handle.second);
			}

			vmInstance = nullptr;
		}

		~WrenHandleMap()
		{
		}

	protected:
		std::unordered_map<std::string, WrenHandle*> classHandleMap;
		WrenVM* vmInstance = nullptr;
	};
}
