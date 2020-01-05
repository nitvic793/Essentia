#pragma once

#include "Memory.h"

template<typename Key, typename Val>
class CustomMap
{

};

constexpr uint32 CMinVectorSize = 32;

bool operator<(EntityHandle lhs, EntityHandle rhs);

bool operator<=(EntityHandle lhs, EntityHandle rhs);

//Limited functionality vector 
template<typename T>
class Vector
{
public:
	Vector(IAllocator* allocator = nullptr) :
		buffer(nullptr)
	{
		Reserve(0, allocator);
	}

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
		allocator = v.allocator;
		memset(&v, 0, sizeof(v)); //Reset v
	}

	Vector(Vector const&& v)
	{
		buffer = v.buffer;
		currentIndex = v.currentIndex;
		capacity = v.capacity;
		allocator = v.allocator;
		memset(&v, 0, sizeof(v));
	}

	Vector& operator=(Vector&& v)
	{
		buffer = v.buffer;
		currentIndex = v.currentIndex;
		capacity = v.capacity;
		allocator = v.allocator;
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

	/*
	WARNING:
	Do not call this function if supplied allocator has been used between previous Reserve/SetSize/Constructor call.
	Do not call if Reserve/SetSize/Constructor with size has not been called.
	*/
	void Grow(uint32 byCount)
	{
		capacity += byCount;
		allocator->Alloc(sizeof(T) * byCount); //Allocated memory resides right next to previous allocation, no need to assign buffer.
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

	void Push(T& value)
	{
		assert(currentIndex + 1 < capacity);
		currentIndex++;
		buffer[currentIndex] = value;
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
		auto& val = buffer[currentIndex];
		currentIndex--;
		return val;
	}

	//const T operator[](size_t index) noexcept
	//{
	//	return buffer[index];
	//}

	T& operator[](size_t index) const noexcept
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

	void CopyFrom(const T* data, const uint32 count)
	{
		memcpy(buffer + Size(), data, sizeof(T) * count);
		currentIndex = (int32)(Size() + count - 1);
	}

	void Sort()
	{
		QuickSort(buffer, 0, (int)currentIndex);
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

	int Partition(T* arr, int l, int h)
	{
		T x = arr[h];
		int i = (l - 1);

		for (int j = l; j <= h - 1; j++) {
			if (arr[j] <= x) {
				i++;
				Swap(arr[i], arr[j]);
			}
		}
		Swap(arr[i + 1], arr[h]);
		return (i + 1);
	}

	void QuickSort(T* A, int l, int h)
	{
		if (l < h) {
			/* Partitioning index */
			int p = Partition(A, l, h);
			QuickSort(A, l, p - 1);
			QuickSort(A, p + 1, h);
		}
	}

	void Swap(T& lhs, T& rhs)
	{
		T temp = lhs;
		lhs = rhs;
		rhs = temp;
	}

	T* buffer = nullptr;
	int32 currentIndex = -1;
	int32 capacity = 0;
	IAllocator* allocator = nullptr;
};