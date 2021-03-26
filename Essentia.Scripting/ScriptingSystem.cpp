#include "pch.h"
#include "ScriptingSystem.h"
#include "WrenScriptingSystemImpl.h"
#include <EventTypes.h>
#include <Trace.h>

void ScriptingSystem::Initialize()
{
	systemImpl = Mem::Alloc<ScriptingSystemImpl>();
	systemImpl->InitializeAllocators();
	systemImpl->Initialize();
	es::GEventBus->Subscribe(this, &ScriptingSystem::OnReload);
}

void ScriptingSystem::Update(float deltaTime, float totalTime)
{
	systemImpl->Update(deltaTime, totalTime);
}

void ScriptingSystem::Destroy()
{
	systemImpl->Destroy();
	Mem::Free(systemImpl);
}

void ScriptingSystem::OnReload(ReloadScriptSystemEvent* reloadEvent)
{
	es::Log("Reloading Scripts");
	es::Log("----------------");
	systemImpl->Destroy();
	systemImpl->Initialize();

}
