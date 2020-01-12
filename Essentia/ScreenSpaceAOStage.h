#pragma once

#pragma once

#include "RenderStage.h"

struct AOParams
{
	DirectX::XMFLOAT4X4	Projection;
	DirectX::XMFLOAT4X4	InvProjection;
	DirectX::XMFLOAT4X4	ProjectionTex;
	DirectX::XMFLOAT4	OffsetVectors[14];
	DirectX::XMFLOAT4	BlurWeights[3];
	DirectX::XMFLOAT2	ScreenSize;
	float				OcclusionRadius;
	float				OcclusionFadeStart;
	float				OcclusionFadeEnd;
	float				SurfaceEpsilon;
};

struct AOVSParams
{
	DirectX::XMFLOAT4X4	InvProjection;
};

struct SSAOBlurParams
{
	DirectX::XMFLOAT2 TexOffset; // 1/ScreenSize.x or y
};

class ScreenSpaceAOStage : public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
private:
	DepthTarget				depthTarget;
	SceneRenderTarget		aoRenderTarget;
	SceneRenderTarget		aoBlurIntermediate;
	SceneRenderTarget		aoBlurFinal;
	ConstantBufferView		aoParamsCBV;
	ConstantBufferView		aoVSParamsCBV;

	ConstantBufferView		aoBlurDir1;
	ConstantBufferView		aoBlurDir2;

	DirectX::XMFLOAT4		mOffsets[14];
	AOParams				aoParams;
	ResourceID				randomVectorResourceId;
	ResourceID				randomVecTextureId;

	void					BlurSSAO(uint32 frameIndex);
	void					BuildRandomVectorTexture(ID3D12GraphicsCommandList* commandList);
	void					BuildOffsetVectors();
	Vector<float>			CalcGaussWeights(float sigma);
};

