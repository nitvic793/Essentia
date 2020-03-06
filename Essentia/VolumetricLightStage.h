#pragma once
#include "RenderStage.h"
class VolumetricLightStage :
	public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
private:
	ConstantBufferView	lightAccumCBV;
	SceneRenderTarget lightAccumTarget;
	EntityHandle volumeEntity;
	MeshHandle cubeMesh;
};

