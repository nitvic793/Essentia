#pragma once
#include "Vertex.h"
#include "CommandContext.h"
#include <wrl.h>

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
	void			Initialize(CommandContext* commandContext);
	const MeshView& GetMeshView(MeshHandle handle);
private:
	MeshManager() {};
	CommandContext*			context = nullptr;
	std::vector<MeshData>	meshes; 
	std::vector<MeshBuffer> buffers;
	std::vector<MeshView>	views;
	friend class Renderer;
};