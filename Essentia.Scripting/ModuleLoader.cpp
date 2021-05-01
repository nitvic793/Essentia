#include "pch.h"
#include "ModuleLoader.h"

#include <iostream>
#include <filesystem>
#include <Memory.h>
#include <StringHash.h>

namespace fs = std::filesystem;

constexpr uint32 CMaxTempScriptBufferSize = 128 * 1024; //128KB max file size for script
constexpr const char* CScriptExtension = ".wren";

//file helpers

char* ReadFile(const char* path, IAllocator* allocator)
{
	FILE* file;
	fopen_s(&file, path, "rb");
	if (file == NULL) return NULL;

	// Find out how big the file is.
	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	// Allocate a buffer for it.
	char* buffer = (char*)allocator->Alloc(fileSize + 1);
	if (buffer == NULL)
	{
		fprintf(stderr, "Could not read file \"%s\".\n", path);
	}

	// Read the entire file.
	size_t bytesRead = fread(buffer, 1, fileSize, file);
	if (bytesRead < fileSize)
	{
		fprintf(stderr, "Could not read file \"%s\".\n", path);
	}

	// Terminate the string.
	buffer[bytesRead] = '\0';

	fclose(file);
	return buffer;
}


std::string GetScriptModuleName(std::string path, const char* basePath)
{
	auto index = path.find(basePath);
	String::Replace(path, basePath, "");
	std::replace(path.begin(), path.end(), '/', '.');
	String::Replace(path, ".wren", "");
	return path;
}

void es::ModuleLoader::LoadModules(WrenVM* vm, const char* basePath)
{
	StackAllocator allocator;
	allocator.Initialize(CMaxTempScriptBufferSize, Mem::GetDefaultAllocator());
	for (const auto& entry : fs::recursive_directory_iterator(basePath))
	{
		if (entry.is_regular_file() && entry.path().has_extension())
		{
			if (entry.path().extension() == CScriptExtension)
			{
				auto modulePath = entry.path().generic_string();
				auto marker = allocator.Push();
				auto moduleName = GetScriptModuleName(modulePath, basePath);
				if (moduleName == "main")
				{
					LoadModule(vm, moduleName.c_str(), modulePath.c_str(), &allocator);
				}
				allocator.Pop(marker);
			}
		}
	}

	allocator.Reset();
}

void es::ModuleLoader::LoadModule(WrenVM* vm, const char* basePath, const char* moduleName)
{
	StackAllocator allocator;
	allocator.Initialize(CMaxTempScriptBufferSize, Mem::GetDefaultAllocator());
	for (const auto& entry : fs::recursive_directory_iterator(basePath))
	{
		if (entry.is_regular_file() && entry.path().has_extension())
		{
			if (entry.path().extension() == CScriptExtension)
			{
				auto modulePath = entry.path().generic_string();
				auto marker = allocator.Push();
				auto moduleNameInferred = GetScriptModuleName(modulePath, basePath);
				if (moduleNameInferred == moduleName)
				{
					LoadModule(vm, moduleNameInferred.c_str(), modulePath.c_str(), &allocator);
					allocator.Pop(marker);
					return;
				}
			}
		}
	}

	allocator.Reset();
}

void es::ModuleLoader::LoadModule(WrenVM* vm, const char* moduleName, const char* modulePath, IAllocator* allocator)
{
	auto source = ReadFile(modulePath, allocator);
	WrenInterpretResult result = wrenInterpret(vm, moduleName, source);
}
