#pragma once

#include "Declarations.h"
#include "Mesh.h"
#include "Material.h"
#include "EntityBase.h"
#include "Transform.h"

class EntityManager;
class Renderer;
class MeshManager;
class ResourceManager;
class ShaderResourceManager;
class DeviceResources;
class CommandContext;
class RenderTargetManager;

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

	EngineContext()
	{
		Context = this;
	}
};