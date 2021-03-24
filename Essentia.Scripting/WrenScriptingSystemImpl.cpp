#include "pch.h"
#include "WrenScriptingSystemImpl.h"
#include <Trace.h>
#include <wren.hpp>

static WrenVM* vm = nullptr;

void WriteVMOutput(WrenVM* vm, const char* text) 
{
    es::Log("%s", text);
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

static WrenVM* InitVM()
{
    WrenConfiguration config;
    wrenInitConfiguration(&config);

    config.writeFn = WriteVMOutput;
    config.errorFn = WriteVMErrorOutput;

    config.initialHeapSize = 1024 * 1024 * 8; //8MB
    return wrenNewVM(&config);
}

void ScriptingSystemImpl::Initialize()
{
    vm = InitVM();
}

void ScriptingSystemImpl::Update(float dt, float totalTime)
{
    WrenInterpretResult result = wrenInterpret(
        vm,
        "my_module",
        "System.print(\"I am running in a VM!\")");
}

void ScriptingSystemImpl::Destroy()
{
    wrenFreeVM(vm);
}
