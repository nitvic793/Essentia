#pragma once
#include "RenderStage.h"
#include "ImguiConsole.h"
#include "EventTypes.h"

class ImguiRenderStage : public IRenderStage
{
public:
	virtual void Initialize() override;
	virtual void Render(const uint32 frameIndex, const FrameContext& frameContext) override;
	virtual void CleanUp() override;
	void OnSelectEntity(SelectEntityEvent* event);
private:
	DescriptorHeap	imguiHeap;
	bool			show = false;
	ImguiConsole	console;
	EntityHandle	selectedEntity = { Handle{CRootParentEntityIndex} };
};

