#pragma once

#include <stdint.h>

// Typedefs

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

typedef uint32 RenderTargetID;
typedef uint32 DepthStencilID;

// Engine Constants

constexpr uint32 CFrameBufferCount = 3; // Triple buffered
constexpr uint32 CFrameMaxDescriptorHeapCount = 2048; // Number of descriptors in the main GPU facing descriptor heap
constexpr uint32 CMaxTextureCount = 512; // Max number of textures
constexpr uint32 CMaxConstantBufferCount = 512; // Max count of constant buffers
constexpr uint64 CMaxConstantBufferSize = 1024 * 256; //256KB

constexpr uint32 CMaxD3DResources = 1024; // Max D3D12 resources.
constexpr uint32 CMaxPipelineStates = 96; 
constexpr uint32 CMaxRootSignatures = 24;
constexpr uint32 CRootParentEntityIndex = UINT32_MAX; // Default Root parent index. 

constexpr int32 CMaxRenderTargets = 64;
constexpr int32 CMaxDepthStencils = 16;

// Engine Types

struct ConstantBufferView
{
	uint64		Offset;
	GPUHeapID	Index;
};

struct DataPack
{
	void*	Data;
	uint32	Size;
};

// Offset of resources in GPU Descriptor heap 
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


enum RootParameterSlot {
	RootSigCBVertex0 = 0,
	RootSigCBPixel0,
	RootSigSRVPixel1,
	RootSigSRVPixel2,
	RootSigCBAll1,
	RootSigCBAll2,
	RootSigIBL,
	RootSigUAV0,
	RootSigParamCount
};

struct ScreenSize
{
	int Width;
	int Height;
};

struct SceneRenderTarget
{
	TextureID		Texture;
	ResourceID		Resource;
	RenderTargetID	RenderTarget;
};

struct DepthTarget
{
	TextureID		Texture;
	ResourceID		Resource;
	DepthStencilID	DepthStencil;
};

struct ColorValues
{
	static constexpr float ClearColor[] = { 0,0,0,1 };
};