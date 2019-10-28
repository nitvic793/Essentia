#pragma once

#define DLL_EXPORT __declspec(dllexport)

class SystemManager;
class IAllocator;
struct EngineContext;

extern "C"
{
	DLL_EXPORT void		Initialize(EngineContext* context);
	DLL_EXPORT void*	CreateGame(IAllocator* allocator);
	DLL_EXPORT void		LoadSystems(SystemManager* systemManager, IAllocator* allocator);
}