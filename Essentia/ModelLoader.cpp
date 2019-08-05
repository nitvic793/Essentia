#include "pch.h"
#include "ModelLoader.h"
#include <unordered_map>

using namespace DirectX;

bool operator==(const XMFLOAT3& l, const XMFLOAT3& r)
{
	return l.x == r.x && l.y == r.y && l.z == r.z;
}

bool operator==(const XMFLOAT2& l, const XMFLOAT2& r)
{
	return l.x == r.x && l.y == r.y;
}

bool operator==(const Vertex& v1, const Vertex& v2) {
	return v1.Position == v2.Position && v1.Normal == v2.Normal && v1.UV == v2.UV;
}

namespace std {

	template <> struct hash<XMFLOAT3>
	{
		size_t operator()(const XMFLOAT3& x) const
		{
			return ((hash<float>()(x.x) ^
				(hash<float>()(x.y) << 1)) >> 1) ^
				(hash<float>()(x.z) << 1);
		}
	};

	template <> struct hash<XMFLOAT2>
	{
		size_t operator()(const XMFLOAT2& x) const
		{
			return (hash<float>()(x.x) ^
				(hash<float>()(x.y) << 1));
		}
	};

	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<XMFLOAT3>()(vertex.Position) ^
				(hash<XMFLOAT3>()(vertex.Normal) << 1)) >> 1) ^
				(hash<XMFLOAT2>()(vertex.UV) << 1);
		}
	};
}

void CalculateTangents(Vertex* vertices, UINT vertexCount, uint32* indices, UINT indexCount)
{
	XMFLOAT3* tan1 = new XMFLOAT3[vertexCount * 2];
	XMFLOAT3* tan2 = tan1 + vertexCount;
	ZeroMemory(tan1, vertexCount * sizeof(XMFLOAT3) * 2);
	int triangleCount = indexCount / 3;
	for (UINT i = 0; i < indexCount; i += 3)
	{
		int i1 = indices[i];
		int i2 = indices[i + 2];
		int i3 = indices[i + 1];
		auto v1 = vertices[i1].Position;
		auto v2 = vertices[i2].Position;
		auto v3 = vertices[i3].Position;

		auto w1 = vertices[i1].UV;
		auto w2 = vertices[i2].UV;
		auto w3 = vertices[i3].UV;

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;
		float r = 1.0F / (s1 * t2 - s2 * t1);

		XMFLOAT3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);

		XMFLOAT3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		XMStoreFloat3(&tan1[i1], XMLoadFloat3(&tan1[i1]) + XMLoadFloat3(&sdir));
		XMStoreFloat3(&tan1[i2], XMLoadFloat3(&tan1[i2]) + XMLoadFloat3(&sdir));
		XMStoreFloat3(&tan1[i3], XMLoadFloat3(&tan1[i3]) + XMLoadFloat3(&sdir));

		XMStoreFloat3(&tan2[i1], XMLoadFloat3(&tan2[i1]) + XMLoadFloat3(&tdir));
		XMStoreFloat3(&tan2[i2], XMLoadFloat3(&tan2[i2]) + XMLoadFloat3(&tdir));
		XMStoreFloat3(&tan2[i3], XMLoadFloat3(&tan2[i3]) + XMLoadFloat3(&tdir));
	}

	for (UINT a = 0; a < vertexCount; a++)
	{
		auto n = vertices[a].Normal;
		auto t = tan1[a];

		// Gram-Schmidt orthogonalize
		auto dot = XMVector3Dot(XMLoadFloat3(&n), XMLoadFloat3(&t));
		XMStoreFloat3(&vertices[a].Tangent, XMVector3Normalize(XMLoadFloat3(&t) - XMLoadFloat3(&n) * dot));

		// Calculate handedness
		//vertices[a].Tangent.w = (XMVectorGetX(XMVector3Dot(XMVector3Cross(XMLoadFloat3(&n), XMLoadFloat3(&t)), XMLoadFloat3(&tan2[a]))) < 0.0F) ? -1.0F : 1.0F;
	}

	delete[] tan1;
}

void ProcessMesh(UINT index, aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<uint32>& indices, std::unordered_map<Vertex, uint32>& uniqueVertices)
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
	std::unordered_map<Vertex, uint32> uniqueVertices = {};

	for (uint32 i = 0; i < pScene->mNumMeshes; ++i)
	{
		ProcessMesh(i, pScene->mMeshes[i], pScene, vertices, indices, uniqueVertices);
	}

	CalculateTangents(vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size());

	mesh.Vertices = std::move(vertices);
	mesh.Indices = std::move(indices);
	return mesh;
}

std::vector<MeshData>  ModelLoader::LoadModel(const std::string& filename)
{
	static Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded | aiProcess_ValidateDataStructure | aiProcess_JoinIdenticalVertices);

	if (pScene == NULL)
		return std::vector<MeshData>();

	std::vector<Vertex> vertices;
	std::vector<uint32> indices;
	std::unordered_map<Vertex, uint32> uniqueVertices = {};
	std::vector<MeshData> meshes(pScene->mNumMeshes);
	for (uint32 i = 0; i < pScene->mNumMeshes; i++)
	{
		MeshData mesh = {};
		ProcessMesh(i, pScene->mMeshes[i], pScene, vertices, indices, uniqueVertices);
		CalculateTangents(vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size());
		mesh.Vertices = std::move(vertices);
		mesh.Indices = std::move(indices);
		meshes[i] = mesh;
	}

	return meshes;
}
