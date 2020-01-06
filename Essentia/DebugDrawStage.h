#pragma once

#include "RenderStage.h"
#include "Utility.h"
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include "Effects.h"
#include <memory>

class DebugDrawStage : public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void CleanUp() override;
private:
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> batch;
	std::unique_ptr<DirectX::BasicEffect> effect;
	ID3D12Device* device;
};
