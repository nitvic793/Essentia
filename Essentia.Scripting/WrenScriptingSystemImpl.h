#pragma once

#include <FreeListAllocator.h>

class ScriptingSystemImpl
{
public:
	void Initialize(const char* basePath);
	void Update(float dt, float totalTime);
	void Destroy();
	void InitializeAllocators();
protected:
	FreeListAllocator allocator;
};