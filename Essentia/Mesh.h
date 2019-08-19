#pragma once
#include "Vertex.h"
#include "CommandContext.h"
#include <wrl.h>
#include <DirectXCollision.h>
#include "ShaderResourceManager.h"

struct MeshData
{
	std::vector<Vertex> Vertices;
	std::vector<uint32> Indices;
};

struct MeshBuffer
{
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer;
};

struct MeshView
{
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	uint32 IndexCount;
};

union MeshHandle
{
	uint32 Id;
};

class MeshManager
{
public:
	MeshHandle		CreateMesh(const std::string& filename, MeshView& meshView);
	MeshHandle		CreateMesh(const MeshData& meshData, MeshView& meshView);
	void			Initialize(CommandContext* commandContext);
	const MeshView& GetMeshView(MeshHandle handle);
	const MeshView& GetMeshView(const char* filename); //WARNING: Lazy loads the mesh
	MeshHandle		GetMeshHandle(const char* filename); //WARNING: Lazy loads the mesh
private:
	MeshManager() {};
	CommandContext*								context = nullptr;
	std::vector<MeshData>						meshes; 
	std::vector<MeshBuffer>						buffers;
	std::vector<MeshView>						views;
	std::vector<DirectX::BoundingOrientedBox>	bounds;
	std::unordered_map<StringID, uint32_t>		meshMap;
	friend class Renderer;
};

union ModelHandle
{
	uint32 Id;
};

// TODO: Load all submeshes in a model in one buffer and then index into the buffer while drawing
struct Model
{
	std::vector<MeshHandle>		Meshes;
	std::vector<MaterialHandle> Materials;
};

class ModelManager
{
public:
	void			Initialize(ShaderResourceManager* srManager);
	ModelHandle		CreateModel(const char* filename);
	const Model&	GetModel(ModelHandle model);
private:
	std::vector<Model> models;
	ShaderResourceManager* shaderResourceManager;
};

