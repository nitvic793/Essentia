#include "pch.h"
#include "ScriptingSystem.h"
#include "WrenScriptingSystemImpl.h"

void ScriptingSystem::Initialize()
{
	systemImpl = Mem::Alloc<ScriptingSystemImpl>();
	systemImpl->Initialize();
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
