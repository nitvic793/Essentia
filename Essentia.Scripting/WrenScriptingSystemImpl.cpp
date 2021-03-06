#include "pch.h"
#include "WrenScriptingSystemImpl.h"
#include "Memory.h"
#include <Trace.h>
#include <wren.hpp>
#include "ModuleLoader.h"
#include <sstream>
#include <StringHash.h>
#include <FreeListAllocator.h>

//Binding Includes
#include "Binding.h"
#include "BaseBindings.h"

//#include <vld.h>

static WrenVM* vm = nullptr;
static char* source = nullptr;
static WrenHandle* updateHandle = nullptr;
static WrenHandle* mainClass = nullptr;
static WrenHandle* initHandle = nullptr;
static WrenHandle* registerControllerHandle = nullptr;

constexpr uint64 CMaxScriptBufferSize = 1 * 1024 * 1024; // 16MB
static IAllocator* sgAllocator = nullptr;
static std::vector<void*> allocs;
static const char* SBasePath = nullptr; // populated on script system init


void WriteVMOutput(WrenVM* vm, const char* text)
{
	if (strcmp(text, "\n") == 0) return; // Ignore new lines as logging system logs each line on its own new line
	es::Log("[info] %s", text);
}

void WriteVMErrorOutput(WrenVM* vm, WrenErrorType errorType,
	const char* module, const int line,
	const char* msg)
{
	switch (errorType)
	{
	case WREN_ERROR_COMPILE:
	{
		es::Log("[error] [%s line %d] %s\n", module, line, msg);
	} break;
	case WREN_ERROR_STACK_TRACE:
	{
		es::Log("[error][%s line %d] in %s\n", module, line, msg);
	} break;
	case WREN_ERROR_RUNTIME:
	{
		es::Log("[error][Runtime Error] %s\n", msg);
	} break;
	}
}


void* ReAllocateVM(void* memory, size_t newSize, void* userData)
{
	if (newSize == 0)
	{
		sgAllocator->Free((byte*)memory);
		return nullptr;
	}

	if (memory == nullptr)
	{
		return sgAllocator->Alloc(newSize);
	}

	void* newBuffer = sgAllocator->Alloc(newSize);
	memcpy(newBuffer, memory, newSize);
	sgAllocator->Free((byte*)memory);
	return newBuffer;
}

void OnLoadModuleComplete(WrenVM* vm, const char* name, struct WrenLoadModuleResult result)
{
}

WrenLoadModuleResult LoadVMModule(WrenVM* vm, const char* name)
{
	std::string path(name);
	String::Replace(path, ".", "/");
	path += ".wren";
	path = SBasePath + path;
	std::ifstream fin;
	fin.open(path, std::ios::in);
	std::stringstream buffer;
	buffer << fin.rdbuf() << '\0';
	std::string source = buffer.str();

	auto vbuffer = sgAllocator->Alloc(source.size());//malloc(source.size());
	//allocs.push_back(vbuffer);
	char* cbuffer = (char*)vbuffer;
	memcpy(cbuffer, source.c_str(), source.size());
	WrenLoadModuleResult result;
	result.source = cbuffer;
	result.onComplete = OnLoadModuleComplete;
	return result;
}

static WrenVM* InitVM()
{
	WrenConfiguration config;
	wrenInitConfiguration(&config);

	config.writeFn = WriteVMOutput;
	config.errorFn = WriteVMErrorOutput;
	config.loadModuleFn = LoadVMModule;
	//config.reallocateFn = ReAllocateVM;
	config.bindForeignMethodFn = es::BindForeignMethod;
	config.bindForeignClassFn = es::BindForeignClass;

	config.initialHeapSize = 1024 * 1024 * 12; //12MB
	return wrenNewVM(&config);
}

int test(int a, int b)
{
	return a + b;
}

void vmTest(WrenVM* vm)
{
	int x = (int)wrenGetSlotDouble(vm, 1);
	int y = (int)wrenGetSlotDouble(vm, 2);
	wrenSetSlotDouble(vm, 0, (double)test(x, y));
}

void CreateBindings()
{
	using namespace es::bindings;

	auto& binding = es::ScriptBinding::GetInstance();
	binding.BindMethod("math.utils", "Utils", "test(_,_)", true, vmTest);
	es::bindings::RegisterBindings();
}

static WrenHandle* controllerHandle = nullptr;
static WrenHandle* controllerUpdateHandle = nullptr;

void ScriptingSystemImpl::Initialize(const char* basePath)
{
	SBasePath = basePath;
	vm = InitVM();

	CreateBindings();
	es::ModuleLoader::LoadModule(vm, basePath, "main");
	es::ModuleLoader::LoadModule(vm, basePath, "controllers.rotationController");

	wrenEnsureSlots(vm, 3);
	wrenGetVariable(vm, "main", "Game", 0);
	mainClass = wrenGetSlotHandle(vm, 0);
	updateHandle = wrenMakeCallHandle(vm, "update(_,_)");
	initHandle = wrenMakeCallHandle(vm, "init()");
	registerControllerHandle = wrenMakeCallHandle(vm, "registerController(_)");
	wrenCall(vm, initHandle);
	wrenReleaseHandle(vm, initHandle);

	wrenEnsureSlots(vm, 3);
	wrenGetVariable(vm, "main", "Game", 0);
	wrenGetVariable(vm, "controllers.rotationController", "RotationController", 1);
	wrenCall(vm, registerControllerHandle);
}

void ScriptingSystemImpl::Update(float dt, float totalTime)
{
	wrenEnsureSlots(vm, 4);
	wrenSetSlotHandle(vm, 0, mainClass);
	wrenSetSlotDouble(vm, 1, dt);
	wrenSetSlotDouble(vm, 2, totalTime);
	auto result = wrenCall(vm, updateHandle);
}

void ScriptingSystemImpl::Destroy()
{
	es::bindings::WrenHandleMap::GetInstance().ReleaseHandles();
	wrenReleaseHandle(vm, mainClass);
	wrenReleaseHandle(vm, updateHandle);
	wrenReleaseHandle(vm, registerControllerHandle);
	wrenFreeVM(vm);

	for (auto& buffer : allocs)
	{
		free(buffer);
	}

	allocs.clear();
	allocator.Reset();
}

void ScriptingSystemImpl::InitializeAllocators()
{
	allocator.Initialize(CMaxScriptBufferSize, Mem::GetDefaultAllocator());
	sgAllocator = &allocator;
}
