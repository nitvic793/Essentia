#pragma once
#include "RenderStage.h"

class SkyBoxRenderStage :
	public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void CleanUp() override;
private:
	ID3D12PipelineState*	skyPSO;
	EntityManager*			entityManager;
	ShaderResourceManager*	shaderResourceManager;
	Renderer*				renderer;
	MeshView				cubeMesh;
};

