#pragma once
#include "Declarations.h"
#include "StringHash.h"

constexpr uint32 CMinFreeIndices = 5;
constexpr uint32 CMaxInitialEntityCount = 128;
constexpr uint32 CMaxInitialComponentCount = 128;

typedef uint32 ComponentTypeID;

struct IComponent {};

#define GComponent(name) \
static const ComponentTypeID Type = crc32(#name);

typedef uint64 Handle;

struct HandleType
{
	uint32 Index;
	uint32 Version;
};

union EntityHandle
{
	HandleType	Handle;
	uint64		ID;
};

union ComponentHandle
{
	HandleType	Handle;
	uint64		ID;
};
