#pragma once
#include "Declarations.h"
#include "StringHash.h"
#include <cereal/archives/json.hpp>

constexpr uint32 CMinFreeIndices = 5;
constexpr uint32 CMaxInitialEntityCount = 128;
constexpr uint32 CMaxInitialComponentCount = 128;

typedef uint32 ComponentTypeID;

struct IComponent
{
	virtual const char* GetName() = 0;
};

#define GComponent(name) \
static const ComponentTypeID Type = crc32(#name); \
virtual const char* GetName() { return #name; }; \
template<class Archive> void serialize(Archive& a){};

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
