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

struct EngineContext
{
	static EngineContext*	Context;
	Renderer*				RendererInstance = nullptr;
	EntityManager*			EntityManager = nullptr;
	MeshManager*			MeshManager = nullptr;
	ShaderResourceManager*	ShaderResourceManager = nullptr;
	ResourceManager*		ResourceManager = nullptr;
	CommandContext*			CommandContext = nullptr;
	DeviceResources*		DeviceResources = nullptr;
	RenderTargetManager*	RenderTargetManager = nullptr;
	ModelManager*			ModelManager = nullptr;
	FrameManager*			FrameManager = nullptr;
	IAllocator*				DefaultAllocator = nullptr;
	IAllocator*				FrameAllocator = nullptr;
	EngineContext()
	{
		Context = this;
	}
};