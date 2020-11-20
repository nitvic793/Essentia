#pragma once
#include "Vertex.h"
#include "CommandContext.h"
#include <wrl.h>
#include <DirectXCollision.h>
#include "ShaderResourceManager.h"
#include "Animation.h"

struct MeshEntry
{
	int NumIndices;
	int BaseVertex;
	int BaseIndex;
};


struct BoneInfo
{
	ogldev::Matrix4f  Offset;
	DirectX::XMFLOAT4X4 OffsetMatrix;
	DirectX::XMFLOAT4X4 FinalTransform;
};


struct AnimationData
{
	std::unordered_map<std::string, uint32_t>	BoneMapping;
	std::vector<BoneInfo>						BoneInfoList;
	std::vector<VertexBoneData>					Bones;
	MeshAnimationDescriptor						Animations;
};

struct MeshData
{
	std::vector<Vertex>		Vertices;
	std::vector<uint32>		Indices;
	std::vector<MeshEntry>	MeshEntries;
	AnimationData			AnimationData;
	bool					IsAnimated = false;
};

struct MeshBuffer
{
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> BoneVertexBuffer;
};

struct MeshView
{
	D3D12_VERTEX_BUFFER_VIEW	VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW		IndexBufferView;
	uint32						IndexCount;
	D3D12_VERTEX_BUFFER_VIEW	BoneVertexBufferView;
	std::vector<MeshEntry>		MeshEntries;
};

union MeshHandle
{
	uint32 Id;
};

class MeshManager
{
public:
	MeshHandle									CreateMesh(const std::string& filename, MeshView& meshView);
	MeshHandle									CreateMesh(const MeshData& meshData, MeshView& meshView);
	MeshHandle									CreateMesh(MeshData& meshData, MeshView& meshView, const char* meshName);
	void										Initialize(CommandContext* commandContext);
	const MeshView&								GetMeshView(MeshHandle handle);
	const MeshView&								GetMeshView(const char* filename); //WARNING: Lazy loads the mesh
	MeshHandle									GetMeshHandle(const char* filename); //WARNING: Lazy loads the mesh
	const DirectX::BoundingBox&					GetBoundingBox(MeshHandle handle);
	std::string									GetName(MeshHandle handle);
	std::vector<std::string>					GetAllMeshNames();
	bool										IsAnimated(MeshHandle mesh) const;
	const AnimationData&						GetAnimationData(MeshHandle mesh);
	void										UpdateMeshData(MeshHandle mesh, const MeshData& meshData);
private:
	MeshManager() {};
	void										CreateBoneBuffers(MeshHandle meshHandle, AnimationData& animData);

	CommandContext*								context = nullptr;
	std::vector<MeshData>						meshes; 
	std::vector<MeshBuffer>						buffers;
	std::vector<MeshView>						views;
	std::vector<DirectX::BoundingBox>			bounds;
	std::unordered_map<uint32, AnimationData>	meshAnimMap; 

	std::unordered_map<StringID, uint32>		meshMap;
	std::unordered_map<uint32, std::string>		meshNameMap;
	friend class Renderer;
};

union ModelHandle
{
	uint32 Id;
};

struct Model
{
	MeshHandle							Mesh;
	std::vector<MaterialHandle>			Materials;
	std::vector<DirectX::BoundingBox>	Bounds;
};

class ModelManager
{
public:
	void			Initialize(ShaderResourceManager* srManager);
	ModelHandle		CreateModel(const char* filename);
	const Model&	GetModel(ModelHandle model);
	ModelHandle		GetModelHandle(const char* name);
	std::string		GetModelName(ModelHandle model);
	std::vector<std::string> GetAllModelNames();
private:
	std::vector<Model>							models;
	std::unordered_map<uint32, std::string>		modelNameMap;
	std::unordered_map<StringID, ModelHandle>	modelMap;
	ShaderResourceManager*						shaderResourceManager;
};

