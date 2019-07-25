#pragma once

#include "Declarations.h"

constexpr uint32 CMinFreeIndices = 5;
constexpr uint32 CMaxInitialEntityCount = 128;
constexpr uint32 CMaxInitialComponentCount = 128;

typedef uint32 ComponentTypeID;

struct IComponent {};

#define GComponent(name) \
static const ComponentTypeID Type = crc32(#name);

struct Handle
{
	uint32 Index;
	uint32 Version;
};

union EntityHandle
{
	Handle Handle;
	uint64 ID;
};

union ComponentHandle
{
	Handle Handle;
	uint64 ID;
};

