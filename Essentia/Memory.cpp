#include "pch.h"
#include "Memory.h"
#include "EngineContext.h"

LinearAllocator* LinearAllocator::Instance = nullptr;

void* Mem::Alloc(size_t sizeInBytes)
{
	return LinearAllocator::Instance->Alloc(sizeInBytes);
}

void Mem::Free(void* buffer)
{
	LinearAllocator::Instance->Free((byte*)buffer);
}


void* SystemHeapAllocator::Alloc(size_t size)
{
	return (void*)new byte[size];
}

void SystemHeapAllocator::Free(byte* buffer)
{
	delete[] buffer;
}
