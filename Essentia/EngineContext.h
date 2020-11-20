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
class SystemManager;
class GameStateManager;
class TerrainManager;
class ConstantBufferViewPool;
struct ImguiConsole;

namespace es
{
	class EventBus;
}


struct EngineContext;
extern EngineContext* GContext;

struct EngineContext
{
	static EngineContext*	Context;
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
	SystemManager*			CoreSystemManager = nullptr;
	SystemManager*			GameSystemManager = nullptr;
	GameStateManager*		GameStateManager = nullptr;
	TerrainManager*			TerrainManager = nullptr;
	ConstantBufferViewPool* ConstantBufferViewPool = nullptr;
	es::EventBus*			EventBus = nullptr;
	ImguiConsole*			Console = nullptr;

	EngineContext()
	{
		Context = this;
		GContext = this;
	}

	//bool IsPlaying() const
	//{
	//	return bIsPlaying;
	//}
};

