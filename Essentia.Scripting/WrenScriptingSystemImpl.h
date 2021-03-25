#pragma once

#include <FreeListAllocator.h>

class ScriptingSystemImpl
{
public:
	void Initialize();
	void Update(float dt, float totalTime);
	void Destroy();
protected:
	FreeListAllocator allocator;
};