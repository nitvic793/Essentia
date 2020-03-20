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
	SceneRenderTarget lightAccumTarget;
	SceneRenderTarget intermediatelightAccumTarget;
	EntityHandle volumeEntity;
	MeshHandle cubeMesh;
};

