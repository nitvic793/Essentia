#include "pch.h"
#include "Memory.h"


StackAllocator* StackAllocator::Instance = nullptr;

void* Mem::Alloc(size_t sizeInBytes)
{
	return StackAllocator::Instance->Alloc(sizeInBytes);
}

void Mem::Free(void* buffer)
{
	StackAllocator::Instance->Free((byte*)buffer);
}

void* SystemHeapAllocator::Alloc(size_t size)
{
	return (void*)new byte[size];
}

void SystemHeapAllocator::Free(byte* buffer)
{
	delete[] buffer;
}
