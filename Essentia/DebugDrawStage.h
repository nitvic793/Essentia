#pragma once

#include "Mesh.h"
#include "RenderStage.h"

class DebugDrawStage : public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void CleanUp() override;
private:
	MeshView cubeMesh;
};

