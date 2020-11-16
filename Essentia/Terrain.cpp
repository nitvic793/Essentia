#include "pch.h"
#include "Terrain.h"
#include "ImageLoader.h"
#include <DirectXMath.h>
#include <DirectXMesh.h>
#include <EngineContext.h>
#include "Mesh.h"

using namespace DirectX;

static constexpr int MAX_COLOR = 255 * 255 * 255;
using byte = unsigned char;

float GetHeight(int x, int y, int width, const std::vector<byte>& image, float minY, float maxY)
{
	byte r = image.at((size_t)x * 4 + 0 + (size_t)y * 4 * (size_t)width);
	byte g = image.at((size_t)x * 4 + 1 + (size_t)y * 4 * (size_t)width);
	byte b = image.at((size_t)x * 4 + 2 + (size_t)y * 4 * (size_t)width);
	byte a = image.at((size_t)x * 4 + 3 + (size_t)y * 4 * (size_t)width);

	int argb = ((0xFF & a) << 24) | ((0xFF & r) << 16)
		| ((0xFF & g) << 8) | (0xFF & b);
	return minY + abs(maxY - minY) * ((float)r / (float)MAX_COLOR);
}

MeshHandle TerrainManager::CreateTerrainMesh(const char* heightMapFile, float minY, float maxY)
{
	uint32 width;
	uint32 height;
	int textInc = 40;
	std::vector<byte> imageData = es::image::LoadPngImage(heightMapFile, width, height);

	float incx = abs(-STARTX * 2) / (width - 1);
	float incz = abs(STARTZ * 2) / (height - 1);

	std::vector<Vertex> vertices;

	std::vector<XMFLOAT2> uvs;
	std::vector<XMFLOAT3> pos;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT3> tangents;
	std::vector<uint32> indices;

	for (uint32 row = 0; row < height; ++row)
	{
		for (uint32 col = 0; col < width; ++col)
		{
			Vertex v;
			XMFLOAT3 pos = {};
			pos.x = STARTX + col * incx;
			pos.y = GetHeight(col, row, width, imageData, minY, maxY);
			pos.z = STARTZ + row * incx;

			XMFLOAT2 uv;
			uv.x = (float)textInc * (float)col / (float)width;
			uv.y = (float)textInc * (float)row / (float)height;
			uvs.push_back(uv);

			v.Position = pos;
			v.UV = uv;
			v.Normal = XMFLOAT3{};

			vertices.push_back(v);

			if (col < width - 1 && row < height - 1)
			{
				int leftTop = row * width + col;
				int leftBottom = (row + 1) * width + col;
				int rightBottom = (row + 1) * width + col + 1;
				int rightTop = row * width + col + 1;

				indices.push_back(rightTop);
				indices.push_back(leftBottom);
				indices.push_back(leftTop);

				indices.push_back(rightBottom);
				indices.push_back(leftBottom);
				indices.push_back(rightTop);
			}
		}
	}

	for (auto& v : vertices)
	{
		pos.push_back(v.Position);
		normals.push_back(v.Normal);
		tangents.push_back(v.Tangent);
		uvs.push_back(v.UV);
	}

	DirectX::ComputeNormals(indices.data(), (size_t)width * height * 2, pos.data(), pos.size(), CNORM_DEFAULT, normals.data());
	DirectX::ComputeTangentFrame(indices.data(), (size_t)width * height * 2, pos.data(), normals.data(), uvs.data(), pos.size(), tangents.data(), nullptr);

	for (int i = 0; i < vertices.size(); ++i)
	{
		vertices[i].Normal = normals[i];
		vertices[i].Tangent = tangents[i];
	}

	MeshEntry entry;
	entry.BaseIndex = 0;
	entry.BaseVertex = 0;
	entry.NumIndices = (int32)indices.size();

	MeshData meshData = {};
	meshData.MeshEntries.push_back(entry);
	meshData.Vertices = vertices;
	meshData.Indices = indices;
	meshData.IsAnimated = false;

	MeshView view;
	MeshHandle handle = GContext->MeshManager->CreateMesh(meshData, view, heightMapFile);

	return handle;
}
