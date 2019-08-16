#pragma once

#include "Memory.h"

template<typename Key, typename Val>
class CustomMap
{

};

constexpr uint32 CMinVectorSize = 32;

template<typename T>
class Vector
{
public:
	Vector(uint32 count = CMinVectorSize, IAllocator* allocator = StackAllocator::Instance)
	{
		capacity = count;
		buffer = Mem::Alloc(sizeof(T) * capacity);
	}

	Vector<T> Move()
	{
		Vector<T> vector = *this;
		capacity = 0;
		currentIndex = -1;
		buffer = nullptr;
	}

	void Push(const T& value)
	{
		assert(currentIndex + 1 < capacity);
		currentIndex++;
		buffer[currentIndex] = value;
	}

	const T& Pop()
	{
		auto val = buffer[currentIndex];
		currentIndex--;
		return val;
	}

	const T& operator[](uint32 index)
	{
		return buffer[index];
	}

	size_t Size()
	{
		return currentIndex + 1;
	}

	~Vector()
	{
		Mem::Free(buffer);
	}

private:
	T* buffer = nullptr;
	int32 currentIndex = -1;
	int32 capacity;

};