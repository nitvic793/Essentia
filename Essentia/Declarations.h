#pragma once

#include <stdint.h>

//Typedefs
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8; 
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint32 GPUHeapID;
typedef uint32 TextureID;
typedef uint32 PipelineStateID;
typedef uint32 RootSignatureID;
typedef uint32 ResourceID;

//Engine Constants
constexpr uint32 CFrameBufferCount = 3;
constexpr uint32 CFrameMaxDescriptorHeapCount = 2048;
constexpr uint32 CMaxTextureCount = 512;
constexpr uint32 CMaxConstantBufferCount = 512;
constexpr uint64 CMaxConstantBufferSize = 1024 * 4; //4KB

constexpr uint32 CMaxD3DResources = 1024;
constexpr uint32 CMaxPipelineStates = 96;
constexpr uint32 CMaxRootSignatures = 24;

struct ConstantBufferView
{
	uint64		Offset;
	GPUHeapID	Index;
};

struct DataPack
{
	void* Data;
	uint32	Size;
};

struct GPUHeapOffsets
{
	uint32 ConstantBufferOffset;
	uint32 TexturesOffset;
	uint32 MaterialsOffset;
};

enum TextureType
{
	WIC,
	DDS
};