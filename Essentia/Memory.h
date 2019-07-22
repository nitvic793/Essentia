#pragma once
#include "Declarations.h"

class IAllocator
{
public:
	virtual void* Alloc(size_t size) = 0;
};


class StackAllocator : public IAllocator
{
public:
	explicit StackAllocator(size_t sizeInBytes)
	{
		buffer = new byte[sizeInBytes];
		current = buffer;
	}

	virtual void* Alloc(size_t size)
	{
		byte* alloc = current;
		current += size;
		return alloc;
	}

	template<typename T, typename ...Args>
	T* Alloc(Args...)
	{
		auto alloc = Alloc(sizeof(T));
		return new(alloc) T(Args...);
	}

	void Free(byte* buff)
	{
		current = buff;
	}

	template<typename T>
	void Free(T* buff)
	{
		buff->~T();
		Free(buff);
	}

	void Clear()
	{
		current = buffer;
	}


	~StackAllocator()
	{
		delete[] buffer;
	}

private:
	byte* buffer = nullptr;
	byte* current = nullptr;
	size_t totalSize = 0;
};

