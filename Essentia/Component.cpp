#include "pch.h"
#include "Component.h"

void ComponentManager::Initialize(IAllocator* allocator)
{
	this->allocator = allocator;
}
