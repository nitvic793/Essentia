#include "pch.h"
#include "Memory.h"


StackAllocator* StackAllocator::Instance = nullptr;

void* Mem::Alloc(size_t sizeInBytes)
{
	return StackAllocator::Instance->Alloc(sizeInBytes);
}

void Mem::Free(void* buffer)
{
	StackAllocator::Instance->Free(buffer);
}
