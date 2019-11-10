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
	Vector() :
		buffer(nullptr) {}

	Vector(uint32 count, IAllocator* allocator = nullptr) :
		buffer(nullptr)
	{
		Reserve(count, allocator);
	}

	Vector(Vector&& v)
	{
		buffer = v.buffer;
		currentIndex = v.currentIndex;
		capacity = v.capacity;
		memset(&v, 0, sizeof(v)); //Reset v
	}

	Vector(const Vector& v) = delete;

	Vector& operator=(Vector&& v)
	{
		buffer = v.buffer;
		currentIndex = v.currentIndex;
		capacity = v.capacity;
		memset(&v, 0, sizeof(v)); //Reset v
		return *this;
	}

	void Reserve(uint32 count = CMinVectorSize, IAllocator* allocator = nullptr)
	{
		this->allocator = allocator;
		if (!allocator)
		{
			this->allocator = Mem::GetDefaultAllocator();
		}

		capacity = count;
		buffer = (T*)this->allocator->Alloc(sizeof(T) * capacity);
	}

	void SetSize(uint32 count = CMinVectorSize, IAllocator* allocator = nullptr)
	{
		Reserve(count, allocator);
		currentIndex = count - 1;
		for (auto& val : *this)
		{
			val = T();
		}
	}

	void Push(T&& value)
	{
		assert(currentIndex + 1 < capacity);
		currentIndex++;
		buffer[currentIndex] = std::move(value);
	}

	const T& Pop()
	{
		assert(currentIndex >= 0);
		auto val = buffer[currentIndex];
		currentIndex--;
		return val;
	}

	//const T&& operator[](size_t index) throw
	//{
	//	return buffer[index];
	//}

	T& operator[](size_t index) noexcept
	{
		return buffer[index];
	}

	constexpr size_t Size()
	{
		return currentIndex + 1;
	}

	constexpr size_t size()
	{
		return currentIndex + 1;
	}

	T* GetData()
	{
		return buffer;
	}

	~Vector()
	{
		if (!buffer) return;
		for (auto& val : *this)
		{
			val.~T();
		}

		if (allocator && buffer)
		{
			allocator->Free((byte*)buffer);
		}
		else if (buffer)
		{
			Mem::Free((void*)buffer);
		}
	}

	T* begin()
	{
		return buffer;
	}

	T* end()
	{
		return buffer + (currentIndex + 1);
	}

private:
	T* buffer = nullptr;
	int32 currentIndex = -1;
	int32 capacity = 0;
	IAllocator* allocator = nullptr;
};