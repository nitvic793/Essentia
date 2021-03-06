#pragma once
#include "RenderStage.h"
#include "VoxelizationStage.h"

struct MipGenParams
{
	DirectX::XMUINT3	OutputResolution;
	float				CurrentMip;
};

class VoxelMipGenStage :
	public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
private:
	ConstantBufferView voxelMipGenCBV[CVoxelGridMips];
};

