#pragma once

#include "RenderStage.h"

class TerrainRenderStage : public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
private:
	Vector<ConstantBufferView> terrainCBVs;
	static constexpr uint32 CMaxTerrainEntityCount = 3;
};