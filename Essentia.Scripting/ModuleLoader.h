#pragma once

#include <wren.hpp>
#include <Memory.h>

namespace es
{
	class ModuleLoader
	{
	public:
		static void LoadModules(WrenVM* vm, const char* basePath);
		static void LoadModule(WrenVM* vm, const char* basePath, const char* moduleName);
	protected:
		static void LoadModule(WrenVM* vm, const char* moduleName, const char* modulePath, IAllocator* allocator);
	};
	
}