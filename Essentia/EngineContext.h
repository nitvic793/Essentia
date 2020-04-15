#pragma once

class EntityManager;
class Renderer;
class MeshManager;
class ResourceManager;
class ShaderResourceManager;
class DeviceResources;
class CommandContext;
class RenderTargetManager;
class ModelManager;
class FrameManager;
class IAllocator;
class ComponentReflector;
class ComputeContext;

struct EngineContext;
extern EngineContext* GContext;

struct EngineContext
{
	static EngineContext*	Context;
	bool					bIsPlaying = false;
	Renderer*				RendererInstance = nullptr;
	EntityManager*			EntityManager = nullptr;
	MeshManager*			MeshManager = nullptr;
	ShaderResourceManager*	ShaderResourceManager = nullptr;
	ResourceManager*		ResourceManager = nullptr;
	CommandContext*			CommandContext = nullptr;
	ComputeContext*			ComputeContext = nullptr;
	DeviceResources*		DeviceResources = nullptr;
	RenderTargetManager*	RenderTargetManager = nullptr;
	ModelManager*			ModelManager = nullptr;
	FrameManager*			FrameManager = nullptr;
	IAllocator*				DefaultAllocator = nullptr;
	IAllocator*				FrameAllocator = nullptr;
	ComponentReflector*		ComponentReflector = nullptr;

	EngineContext()
	{
		Context = this;
		GContext = this;
	}

	bool IsPlaying() const
	{
		return bIsPlaying;
	}
};

