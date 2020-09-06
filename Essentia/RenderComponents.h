#pragma once

#include "EntityBase.h"
#include "ConstantBuffer.h"
#include "Mesh.h"
#include "Engine.h"
#include "CMath.h"

typedef
enum DrawableRenderFlags
{
	kDrawableNoneFlag = 0,
	kDrawableShadowCaster =  1 << 0,
	kDrawableAnimatedMesh = 1 << 1
} DrawableRenderFlags;

inline DrawableRenderFlags operator|(DrawableRenderFlags a, DrawableRenderFlags b)
{
	return static_cast<DrawableRenderFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline DrawableRenderFlags operator|=(DrawableRenderFlags a, DrawableRenderFlags b)
{
	return static_cast<DrawableRenderFlags>(static_cast<int>(a) | static_cast<int>(b));
}

struct IDrawable : public IComponent {};

struct BaseDrawableComponent : public IComponent
{
	ConstantBufferView	CBView;
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4X4 WorldViewProjection;
	DirectX::XMFLOAT4X4 PrevWorldViewProjection;

	static BaseDrawableComponent Create()
	{
		BaseDrawableComponent component;
		component.CBView = es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		return component;
	}

	template<class Archive>
	void save(Archive& archive) const
	{
	};

	template<class Archive>
	void load(Archive& archive)
	{
		*this = Create();
	};

	GComponent(BaseDrawableComponent)
};

struct DrawableComponent : public IDrawable
{
	GComponent(DrawableComponent)
	MeshHandle			Mesh;
	MaterialHandle		Material;
	DrawableRenderFlags	Flags;
	ConstantBufferView	CBView;
	DirectX::XMFLOAT4X4 WorldViewProjection;
	DirectX::XMFLOAT4X4 PrevWorldViewProjection;

	static DrawableComponent Create(MeshHandle mesh, MaterialHandle material)
	{
		DrawableComponent component = {};
		component.CBView = es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.Mesh = mesh;
		component.Material = material;
		component.Flags = kDrawableShadowCaster | GContext->MeshManager->IsAnimated(mesh) ? kDrawableAnimatedMesh : kDrawableNoneFlag;
		return component;
	}

	bool IsAnimated() const
	{
		return Flags & kDrawableAnimatedMesh;
	}

	template<class Archive>
	void save(Archive& archive) const
	{
		auto ec = EngineContext::Context;
		auto MeshName = ec->MeshManager->GetName(Mesh);
		auto MaterialName = ec->ShaderResourceManager->GetMaterialName(Material);
		archive(CEREAL_NVP(MeshName), CEREAL_NVP(MaterialName));
	};

	template<class Archive>
	void load(Archive& archive)
	{
		auto ec = EngineContext::Context;
		std::string MeshName;
		std::string MaterialName;
		archive(CEREAL_NVP(MeshName), CEREAL_NVP(MaterialName));
		auto comp = Create(
			ec->MeshManager->GetMeshHandle(MeshName.c_str()),
			ec->ShaderResourceManager->GetMaterialHandle(MaterialName.c_str())
		);
		*this = comp;
	};
};

// To support models with inbuilt textures/materials
struct DrawableModelComponent : public IDrawable
{
	GComponent(DrawableModelComponent)
		ModelHandle			Model;
	ConstantBufferView	CBView;
	DirectX::XMFLOAT4X4 WorldViewProjection;
	DirectX::XMFLOAT4X4 PrevWorldViewProjection;

	static DrawableModelComponent Create(ModelHandle model)
	{
		DrawableModelComponent component;
		component.CBView = es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.Model = model;
		return component;
	}

	template<class Archive>
	void save(Archive& archive) const
	{
		auto ec = EngineContext::Context;
		auto ModelName = ec->ModelManager->GetModelName(Model);
		archive(CEREAL_NVP(ModelName));
	};

	template<class Archive>
	void load(Archive& archive)
	{
		auto ec = EngineContext::Context;
		std::string ModelName;
		archive(CEREAL_NVP(ModelName));
		auto handle = ec->ModelManager->GetModelHandle(ModelName.c_str());
		auto comp = Create(handle);
		*this = comp;
	};
};

struct ILight : public IComponent {};

struct DirectionalLightComponent : public ILight
{
	GComponent(DirectionalLightComponent)

		DirectX::XMFLOAT3	Color;
	float				Intensity;
	DirectX::XMFLOAT3	Direction;
	float				Padding;

	static DirectionalLightComponent Create(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1, 1, 1), float intensity = 1.f)
	{
		DirectionalLightComponent light;
		light.Color = color;
		light.Direction = direction;
		light.Intensity = intensity;
		return light;
	}

	DirectionalLight GetLight()
	{
		return { Direction, 0, Color, Intensity };
	}

	template<class Archive>
	void save(Archive& archive) const
	{
		Vector3 Color(this->Color);
		Vector3 Direction(this->Direction);
		archive(
			CEREAL_NVP(Color),
			CEREAL_NVP(Intensity),
			CEREAL_NVP(Direction)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		Vector3 Color;
		Vector3 Direction;
		archive(
			CEREAL_NVP(Color),
			CEREAL_NVP(Intensity),
			CEREAL_NVP(Direction)
		);

		*this = Create((DirectX::XMFLOAT3)Direction, (DirectX::XMFLOAT3)Color, Intensity);
	};
};

struct PointLightComponent : public ILight
{
	DirectX::XMFLOAT3	Color;
	float				Intensity;
	float				Range;
	float				Padding[3];

	static PointLightComponent Create(const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1, 1, 1), float intensity = 1.f, float range = 5.f)
	{
		PointLightComponent light;
		light.Color = color;
		light.Range = range;
		light.Intensity = intensity;
		return light;
	}

	PointLight GetLight()
	{
		return { {}, Intensity, Color, Range };
	}


	template<class Archive>
	void save(Archive& archive) const
	{
		Vector3 Color(this->Color);
		archive(
			CEREAL_NVP(Color),
			CEREAL_NVP(Intensity),
			CEREAL_NVP(Range)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		Vector3 Color;
		archive(
			CEREAL_NVP(Color),
			CEREAL_NVP(Intensity),
			CEREAL_NVP(Range)
		);

		*this = Create((DirectX::XMFLOAT3)Color, Intensity, Range);
	};

	GComponent(PointLightComponent)
};

struct SpotLightComponent : public ILight
{
	DirectX::XMFLOAT3	Color;
	float				Intensity;
	float				Range;
	DirectX::XMFLOAT3	Direction;
	float				SpotFalloff;

	static SpotLightComponent Create(
		const DirectX::XMFLOAT3& direction,
		const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1, 1, 1),
		float intensity = 1.f,
		float range = 5.f,
		float spotFalloff = 10.f
	)
	{
		SpotLightComponent light;
		light.Direction = direction;
		light.Color = color;
		light.Range = range;
		light.Intensity = intensity;
		light.SpotFalloff = spotFalloff;
		return light;
	}

	SpotLight GetLight()
	{
		return { Color, Intensity, {}, Range, Direction, SpotFalloff };
	}

	template<class Archive>
	void save(Archive& archive) const
	{
		Vector3 Color(this->Color);
		Vector3 Direction(this->Direction);
		archive(
			CEREAL_NVP(Color),
			CEREAL_NVP(Intensity),
			CEREAL_NVP(Range),
			CEREAL_NVP(Direction),
			CEREAL_NVP(SpotFalloff)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		Vector3 Color;
		Vector3 Direction;
		archive(
			CEREAL_NVP(Color),
			CEREAL_NVP(Intensity),
			CEREAL_NVP(Range),
			CEREAL_NVP(Direction),
			CEREAL_NVP(SpotFalloff)
		);

		*this = Create(
			(DirectX::XMFLOAT3)Direction,
			(DirectX::XMFLOAT3)Color,
			Intensity,
			Range,
			SpotFalloff
		);
	};

	GComponent(SpotLightComponent)
};

struct SkyboxComponent : public IComponent
{
	GComponent(SkyboxComponent)
		TextureID			CubeMap;
	ConstantBufferView	CBView;

	static SkyboxComponent Create(const char* skyboxFileName, TextureType type = DDS)
	{
		SkyboxComponent component;
		component.CBView = es::CreateConstantBufferView(sizeof(PerObjectConstantBuffer));
		component.CubeMap = es::CreateTexture(skyboxFileName, type, false);
		return component;
	}

	template<class Archive>
	void save(Archive& archive) const
	{
		auto ec = EngineContext::Context;
		auto CubeMapName = ec->ShaderResourceManager->GetTextureName(CubeMap);
		archive(CEREAL_NVP(CubeMapName));
	};

	template<class Archive>
	void load(Archive& archive)
	{
		auto ec = EngineContext::Context;
		std::string CubeMapName;
		archive(CEREAL_NVP(CubeMapName));
		auto comp = Create(CubeMapName.c_str());
		*this = comp;
	};
};

// Add this component to any entity which is to be selected
struct SelectedComponent : public IComponent
{
	GComponent(SelectedComponent)

		template<class Archive> void serialize(Archive& a) {};
};