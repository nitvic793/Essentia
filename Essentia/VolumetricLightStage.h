#pragma once
#include "RenderStage.h"
class VolumetricLightStage :
	public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
private:
	void BilateralBlur(int frameIndex);

	ConstantBufferView	lightAccumCBV;
	ConstantBufferView	bilateralBlurCBV;
	ConstantBufferView	blurHorizontalCBV;
	ConstantBufferView	blurVerticalCBV;
	SceneRenderTarget lightAccumTarget;
	SceneRenderTarget intermediatelightAccumTarget;
	SceneRenderTarget blurFinalTarget;
	SceneRenderTarget blurIntermediateTarget;
	EntityHandle volumeEntity;
	MeshHandle cubeMesh;
};

