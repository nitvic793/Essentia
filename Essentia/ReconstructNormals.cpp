#include "ReconstructNormals.h"
#include "RenderTargetManager.h"
#include "Renderer.h"

void ReconstructNormals::Initialize()
{
	GRenderStageManager.RegisterStage(this);
	auto renderer = GContext->RendererInstance;
	auto screenSize = renderer->GetScreenSize();
	normalTarget = CreateSceneRenderTarget(GContext, screenSize.Width, screenSize.Height, DXGI_FORMAT_R8G8B8A8_SNORM);
}

void ReconstructNormals::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
}

void ReconstructNormals::CleanUp()
{
}
