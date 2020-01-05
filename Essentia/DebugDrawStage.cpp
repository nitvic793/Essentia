#include "pch.h"
#include "DebugDrawStage.h"
#include "DebugDraw.h"
#include "Renderer.h"
#include "EffectPipelineStateDescription.h"
#include "CommonStates.h"


using namespace DirectX;

void DebugDrawStage::Initialize()
{
//	auto ec = EngineContext::Context;
//	device = ec->DeviceResources->GetDevice();
//	batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(device);
//	RenderTargetState rtState(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
//	EffectPipelineStateDescription pd(
//		&VertexPositionColor::InputLayout,
//		CommonStates::Opaque,
//		CommonStates::DepthDefault,
//		CommonStates::CullNone,
//		rtState);
//
//	auto ptr = Mem::Alloc(sizeof(BasicEffect));
//	effect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, pd);
}

void DebugDrawStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	//auto ec = EngineContext::Context;
	//auto commandList = ec->RendererInstance->GetDefaultCommandList();

	//effect->Apply(commandList);

	//batch->Begin(commandList);
	//BoundingBox box;
	//box.Extents = XMFLOAT3(1, 1, 1);
	//box.Center = XMFLOAT3(1, 1, 1);
	//Draw(batch.get(), box);
	//batch->End();
}

void DebugDrawStage::CleanUp()
{
}
