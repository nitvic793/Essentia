#pragma once

#include "Mesh.h"
#include "RenderStage.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "Effects.h"

class DebugDrawStage : public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void CleanUp() override;
private:
	MeshView cubeMesh;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> mBatch;
	std::unique_ptr<DirectX::BasicEffect> mEffect;
};

