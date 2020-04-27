#pragma once
#include "RenderStage.h"


class VoxelizationStage :
	public IRenderStage
{
public:
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void Initialize() override;
private:
	TextureID voxelGrid3dTextureSRV;
	TextureID voxelGrid3dTextureUAV;
};

