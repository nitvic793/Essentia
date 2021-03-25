#pragma once

#include <System.h>
#include <EventTypes.h>

class ScriptingSystemImpl;

class ScriptingSystem : public ISystem
{
public:
	virtual void Initialize();
	virtual void Update(float deltaTime, float totalTime);
	virtual void Destroy();

	void OnReload(ReloadScriptSystemEvent* reloadEvent);

protected:
	ScriptingSystemImpl* systemImpl; 
};