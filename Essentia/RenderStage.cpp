#include "pch.h"
#include "RenderStage.h"

RenderStageManager GRenderStageManager;

void RenderStageManager::RegisterStage(std::string_view stageName, IRenderStage* stage)
{
	renderStages[stageName] = stage;
}

void RenderStageManager::SetEnabled(std::string_view stageName, bool enabled)
{
	renderStages[stageName]->Enabled = enabled;
}

const RenderStageMap& RenderStageManager::GetRenderStageMap()
{
	return renderStages;
}
