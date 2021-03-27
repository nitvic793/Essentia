#pragma once

#include <System.h>
#include <EventTypes.h>
#include <FileWatcher.h>

class ScriptingSystemImpl;

class ScriptingSystem : public ISystem
{
public:
	ScriptingSystem();
	virtual void Initialize();
	virtual void Update(float deltaTime, float totalTime);
	virtual void Destroy();

	void OnReload(ReloadScriptSystemEvent* reloadEvent);

protected:
	ScriptingSystemImpl* systemImpl; 
	ScopedPtr<FileWatcher> fileWatcher;
	std::thread watcherThread;
};