#pragma once
#include "Declarations.h"
#include <utility>
#include "EngineContext.h"

#ifdef DLL_A
#define A_EXPORT __declspec(dllexport)
#else
#define A_EXPORT __declspec(dllimport)
#endif

constexpr uint32 CMaxStackHeapSize = 1024 * 1024 * 64; //64MB
constexpr uint32 CMaxScratchSize = 1024 * 1024 * 16; //16MB

enum MemorySpaceType
{
	kMemStackSpace,
	kMemScratchSpace,
	kMemFrameSpace
};

class IAllocator
{
public:
	virtual void* Alloc(size_t size) = 0;
	virtual void Free(byte* buffer) = 0;
	virtual void Reset() {};
	virtual ~IAllocator() {}
};

class SystemHeapAllocator : public IAllocator
{
public:
	static const SystemHeapAllocator& Get()
	{
		static SystemHeapAllocator allocator = SystemHeapAllocator();
		return allocator;
	}

	virtual void* Alloc(size_t size) override;
	virtual void Free(byte* buffer) override;
};

class LinearAllocator : public IAllocator
{
public:
	static LinearAllocator* Instance;
	explicit LinearAllocator(size_t sizeInBytes, IAllocator* allocator = nullptr)
	{
		if (!allocator)
		{
			allocator = (IAllocator*)&SystemHeapAllocator::Get();
		}
		parent = allocator;
		Instance = this;
		buffer = (byte*)allocator->Alloc(sizeInBytes);
		current = buffer;
		totalSize = sizeInBytes;
		memset(buffer, 0, totalSize);
	}

	virtual void* Alloc(size_t size) override
	{
		assert((current + size) < (buffer + totalSize));
		byte* alloc = current;
		current += size;
		return alloc;
	}

	virtual void Free(byte* buff) override
	{
		//current = buff;
	}

	virtual void Reset() override
	{
		current = buffer;
	}

	~LinearAllocator()
	{
		parent->Free(buffer);
	}

	static LinearAllocator* GetInstance()
	{
		return Instance;
	}

private:
	byte*		buffer = nullptr;
	byte*		current = nullptr;
	size_t		totalSize = 0;
	IAllocator* parent = nullptr;
};

class StackAllocator : public IAllocator
{
public:
	StackAllocator(){}

	void Initialize(size_t sizeInBytes, IAllocator* allocator = nullptr)
	{
		if (!allocator)
		{
			allocator = LinearAllocator::GetInstance();
		}
		parent = allocator;
		buffer = (byte*)allocator->Alloc(sizeInBytes);
		current = buffer;
		totalSize = sizeInBytes;
		marker = buffer;
	}

	virtual void* Alloc(size_t size) override
	{
		assert((current + size) <= (buffer + totalSize));
		byte* alloc = current;
		current += size;
		return alloc;
	}

	virtual void Free(byte* buff) override
	{
		current = buff;
	}

	byte* Push()
	{
		marker = current;
		return marker;
	}

	void Pop(byte* marker)
	{
		current = marker;
	}

	virtual void Reset() override
	{
		current = buffer;
	}

	~StackAllocator()
	{
		parent->Free(buffer);
	}

private:
	byte*		buffer = nullptr;
	byte*		current = nullptr;
	byte*		marker = nullptr;
	size_t		totalSize = 0;
	IAllocator* parent = nullptr;
};


namespace Mem
{
	void*	Alloc(size_t sizeInBytes);
	void	Free(void* buffer);

	static IAllocator* GetDefaultAllocator()
	{
		return LinearAllocator::GetInstance();
	}

	static IAllocator* GetFrameAllocator()
	{
		return GContext->FrameAllocator;
	}

	template<typename T, typename ...Args>
	T* Alloc(Args&& ... args)
	{
		auto alloc = Alloc(sizeof(T));
		return new(alloc) T(std::forward<Args>(args)...);
	}

	template<typename T>
	T* AllocArray(size_t count)
	{
		auto alloc = Alloc(sizeof(T) * count);
		return (T*)alloc;
	}

	template<typename T>
	void Free(T* buff)
	{
		buff->~T();
		Free((void*)buff);
	}
}

template<typename T>
class ScopedPtr
{
public:
	explicit ScopedPtr(T* p)
	{
		ptr = p;
	}

	ScopedPtr(ScopedPtr&& p):
		ptr(p.ptr)
	{
		p.ptr = nullptr;
	}

	ScopedPtr& operator=(const ScopedPtr& rhs) = delete;

	ScopedPtr& operator=(ScopedPtr&& rhs)
	{
		ptr = rhs.ptr;
		rhs.ptr = nullptr;
		return *this;
	}

	ScopedPtr() {}

	T& operator * () { return *ptr; }
	T* operator -> () { return ptr; }
	T* operator -> () const { return ptr; }
	T* Get() { return ptr; }
	T* Get() const { return ptr; }
	//For compatibility
	T* get() { return ptr; }
	T* get() const { return ptr; }

	~ScopedPtr()
	{
		if (ptr)
		{
			ptr->~T();
			ptr = nullptr;
		}
	}
private:
	T* ptr = nullptr;
};

template<typename T>
static ScopedPtr<T> MakeScoped()
{
	auto allocator = LinearAllocator::GetInstance();
	return MakeScoped<T>(allocator);
}

template<typename T>
static ScopedPtr<T> MakeScoped(IAllocator* allocator)
{
	auto alloc = allocator->Alloc(sizeof(T));
	T* p = new(alloc) T();
	ScopedPtr<T> ptr(p);
	return ptr;
}

template<typename T, typename ...Args>
static ScopedPtr<T> MakeScopedArgs(Args&& ... args)
{
	auto allocator = LinearAllocator::GetInstance();
	auto alloc = allocator->Alloc(sizeof(T));
	ScopedPtr<T> ptr(new(alloc) T(std::forward<Args>(args)...));
	return ptr;
//	return MakeScopedArgsAlloc(std::forward<Args>(args)..., allocator);
}

template<typename T, typename ...Args>
static ScopedPtr<T> MakeScopedArgsAlloc(Args&&... args, IAllocator* allocator)
{
	auto alloc = allocator->Alloc(sizeof(T));
	ScopedPtr<T> ptr(new(alloc) T(std::forward<Args>(args)...));
	return ptr;
}