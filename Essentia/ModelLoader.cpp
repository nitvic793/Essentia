#include "pch.h"
#include "ModelLoader.h"
#include <unordered_map>

using namespace DirectX;
//
//bool operator
//
//bool operator==(const Vertex& v1, const Vertex& v2) const {
//	return v1.Position == v2.Position && color == other.color && texCoord == other.texCoord;
//}
//
//namespace std {
//	template<> struct hash<Vertex> {
//		size_t operator()(Vertex const& vertex) const {
//			return ((hash<glm::vec3>()(vertex.pos) ^
//				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
//				(hash<glm::vec2>()(vertex.texCoord) << 1);
//		}
//	};
//}

void ProcessMesh(UINT index, aiMesh* mesh, const aiScene* scene, std::vector<MeshEntry> meshEntries, std::vector<Vertex>& vertices, std::vector<uint32>& indices, std::unordered_map</*Vertex*/int, uint32>& uniqueVertices)
{
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.Position.x = mesh->mVertices[i].x;
		vertex.Position.y = mesh->mVertices[i].y;
		vertex.Position.z = mesh->mVertices[i].z;

		vertex.Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		if (mesh->mTextureCoords[0])
		{
			vertex.UV.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.UV.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
}


MeshData ModelLoader::Load(const std::string& filename)
{
	MeshData mesh;
	static Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded | aiProcess_ValidateDataStructure | aiProcess_JoinIdenticalVertices);

	if (pScene == NULL)
		return mesh;

	uint32 NumVertices = 0;
	uint32 NumIndices = 0;
	std::vector<MeshEntry> meshEntries(pScene->mNumMeshes);
	for (uint32 i = 0; i < meshEntries.size(); i++)
	{
		meshEntries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		meshEntries[i].BaseVertex = NumVertices;
		meshEntries[i].BaseIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += meshEntries[i].NumIndices;
	}

	std::vector<Vertex> vertices;
	std::vector<uint32> indices;
	std::unordered_map</*Vertex*/int, uint32> uniqueVertices = {};

	for (uint32 i = 0; i < pScene->mNumMeshes; ++i)
	{
		ProcessMesh(i, pScene->mMeshes[i], pScene, meshEntries, vertices, indices, uniqueVertices);
	}

	mesh.Vertices = std::move(vertices);
	mesh.Indices = std::move(indices);
	return mesh;
}
