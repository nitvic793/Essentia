#pragma once

#include "FrameContext.h"

enum RenderStageType
{
	eRenderStageMain = 0,
	eRenderTypeTransparent,
	eRenderStageGUI,
	eRenderTypeCount
};

//TODO: Need to pass info from one stage to another -> Render Target and other info?
//Assumptions: Render Target set by renderer is always the default render target
class IRenderStage
{
public:
	virtual void Initialize() {};
	virtual void Clear() {};
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) = 0;
	virtual void CleanUp() {};
	virtual ~IRenderStage() {};
	bool Enabled = true;
private:
};

using RenderStageMap = std::unordered_map<std::string_view, IRenderStage*>;

class RenderStageManager
{
public:
	void RegisterStage(std::string_view stageName, IRenderStage* stage);
	void SetEnabled(std::string_view stageName, bool enabled);
	const RenderStageMap& GetRenderStageMap();
private:
	RenderStageMap renderStages;
};

extern RenderStageManager GRenderStageManager;