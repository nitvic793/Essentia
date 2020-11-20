#include "TerrainRenderStage.h"
#include "Renderer.h"
#include "Entity.h"
#include "TerrainComponent.h"

void TerrainRenderStage::Initialize()
{
	GRenderStageManager.RegisterStage("TerrainRenderStage", this);
}

void TerrainRenderStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto renderer = GContext->RendererInstance;
	uint32 count;
	auto commandList = renderer->GetDefaultCommandList();
	auto terrains = GContext->EntityManager->GetComponents<TerrainComponent>(count);

	commandList->SetPipelineState(GContext->ResourceManager->GetPSO(GPipelineStates.TerrainPSO));

	assert(count < CMaxTerrainEntityCount);

	for (uint32 i = 0; i < count; ++i)
	{
		MeshHandle mesh = terrains[i].TerrainMesh;
		MaterialHandle material = terrains[i].Material;
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, terrains[i].ConstantBufferView);
		renderer->SetShaderResourceViewMaterial(commandList, RootSigSRVPixel1, material);
		renderer->DrawMesh(commandList, mesh);
	}
}
