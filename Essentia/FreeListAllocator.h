#pragma once

#include <Memory.h>

/*TODO: Aligned allocations*/
/*WIP: NOT usable yet*/
class FreeListAllocator : public IAllocator
{
public:
	struct Block;
	struct Header;

	using BytePtr = byte*;
	using BlockPtr = Block*;
	using HeaderPtr = Header*;

	struct Header
	{
		size_t size;
	};

	struct Block
	{
		size_t		size;
		BlockPtr	next;
	};

	FreeListAllocator(){}

	FreeListAllocator(size_t size, IAllocator* allocator = nullptr)
	{
		Initialize(size, allocator);
	}

	void Initialize(size_t size, IAllocator* allocator = nullptr)
	{
		if (allocator == nullptr)
		{
			allocator = Mem::GetDefaultAllocator();
		}

		parent = allocator;
		buffer = (BytePtr)allocator->Alloc(size);
		freeBlocks = (BlockPtr)buffer;
		freeBlocks->size = size;
		freeBlocks->next = nullptr;
		totalSize = size;
	}

	virtual void* Alloc(size_t size)
	{
		assert(size != 0);
		auto block = freeBlocks;
		BlockPtr prev = nullptr;
		auto headerSize = sizeof(Header);
		auto requiredSize = size + headerSize;

		while (block != nullptr)
		{
			if (block->size < requiredSize)
			{
				prev = block;
				block = block->next;
				continue;
			}

			if (block->size >= requiredSize)
			{
				BlockPtr nextBlock = nullptr;
				if (block->next == nullptr) //End of list
				{
					nextBlock = (BlockPtr)((BytePtr)block + requiredSize); //Calc next free block
					nextBlock->next = nullptr;
					nextBlock->size = block->size - requiredSize;
				}
				else
				{
					nextBlock = block->next;
				}

				freeBlocks = nextBlock;
				freeBlocks->size = nextBlock->size;
				HeaderPtr header = (HeaderPtr)block;
				header->size = requiredSize;
				block = (BlockPtr)((BytePtr)block + headerSize);

				return (void*)block;
			}

			block = block->next;
		}

		return nullptr;
	}

	virtual void Free(byte* buffer)
	{
		if (buffer == nullptr) return;

		HeaderPtr header = (HeaderPtr)(buffer - sizeof(Header));
		auto size = header->size;
		auto block = (BlockPtr)header;
		block->size = size;
		block->next = freeBlocks;
		freeBlocks = block;
	};


	virtual void Reset()
	{
		freeBlocks = (BlockPtr)buffer;
		freeBlocks->size = totalSize;
		freeBlocks->next = nullptr;
	}

	~FreeListAllocator()
	{
		if (parent)
		{
			parent->Free(buffer);
		}
	}
protected:
	BytePtr buffer;
	size_t	totalSize;
	BlockPtr	freeBlocks;
	IAllocator* parent;
};