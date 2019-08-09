#pragma once
#include "Declarations.h"
#include <utility>

constexpr uint32 CMaxStackHeapSize = 1024 * 1024 * 32; //32MB

class IAllocator
{
public:
	virtual void* Alloc(size_t size) = 0;
};

class StackAllocator : public IAllocator
{
public:
	static StackAllocator* Instance;
	explicit StackAllocator(size_t sizeInBytes)
	{
		Instance = this;
		buffer = new byte[sizeInBytes];
		current = buffer;
		totalSize = sizeInBytes;
	}

	virtual void* Alloc(size_t size)
	{
		byte* alloc = current;
		current += size;
		return alloc;
	}

	template<typename T, typename ...Args>
	T* Alloc(Args&&... args)
	{
		auto alloc = Alloc(sizeof(T));
		return new(alloc) T(std::forward<Args>(args)...);
	}

	template<typename T>
	T* Alloc(size_t count)
	{
		auto alloc = Alloc(sizeof(T) * count);
		return (T*)alloc;
	}

	void Free(byte* buff)
	{
		current = buff;
	}

	template<typename T>
	void Free(T* buff)
	{
		buff->~T();
		Free((byte*)buff);
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

namespace Mem
{
	void*	Alloc(size_t sizeInBytes);
	void	Free(void* buffer);
}

template<typename T>
struct Deleter
{
	void operator()(T* buffer)
	{
		StackAllocator::Instance->Free(buffer);
	}
};

template<typename T>
using UniquePtr = std::unique_ptr<T, Deleter<T>>;

template<class T, typename ...Args> 
UniquePtr<T> MakeUnique(Args&&... args)
{
	return std::unique_ptr<T, Deleter<T>>(StackAllocator::Instance->Alloc<T>(std::forward<Args>(args)...));
}
