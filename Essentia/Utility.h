#pragma once

#include "Memory.h"

template<typename Key, typename Val>
class CustomMap
{

};

constexpr uint32 CMinVectorSize = 32;

//Limited functionality vector 
template<typename T>
class Vector
{
public:
	Vector() {}

	Vector(uint32 count, IAllocator* allocator = nullptr)
	{
		SetSize(count, allocator);
	}

	void SetSize(uint32 count = CMinVectorSize, IAllocator* allocator = nullptr)
	{
		if (!allocator)
		{
			allocator = Mem::GetDefaultAllocator();
		}

		capacity = count;
		buffer = (T*)allocator->Alloc(sizeof(T) * capacity);
	}

	void Push(T&& value)
	{
		assert(currentIndex + 1 < capacity);
		currentIndex++;
		buffer[currentIndex] = std::move(value);
	}

	const T& Pop()
	{
		auto val = buffer[currentIndex];
		currentIndex--;
		return val;
	}

	const T& operator[](size_t index)
	{
		return buffer[index];
	}

	constexpr size_t Size()
	{
		return currentIndex + 1;
	}

	T* GetData()
	{
		return buffer;
	}

	~Vector()
	{
		Mem::Free(buffer);
	}

private:
	T* buffer = nullptr;
	int32 currentIndex = -1;
	int32 capacity = 0;

};