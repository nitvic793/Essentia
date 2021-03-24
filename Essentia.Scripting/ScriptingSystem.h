#pragma once

#include <System.h>

class ScriptingSystemImpl;

class ScriptingSystem : public ISystem
{
public:
	virtual void Initialize();
	virtual void Update(float deltaTime, float totalTime);
	virtual void Destroy();

protected:
	ScriptingSystemImpl* systemImpl; 
};