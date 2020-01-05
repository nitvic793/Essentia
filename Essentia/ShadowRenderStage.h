#pragma once
#include "RenderStage.h"

constexpr uint32 CShadowMapSize = 4096;

struct ShadowDirParams
{
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
};

class ShadowRenderStage :
	public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
private:
	DepthTarget shadowDepthTarget;
	Vector<ConstantBufferView> shadowCBVs;
};

