#pragma once
#include "Declarations.h"
#include "EngineContext.h"
#include <unordered_map>

struct PostProcessRenderTarget
{
	TextureID		Texture;
	ResourceID		Resource;
	RenderTargetID	RenderTarget;
};

PostProcessRenderTarget CreatePostProcessRenderTarget(EngineContext* context, uint32 width, uint32 height, DXGI_FORMAT format);

//TODO : Pass the processed screen texture to the next stage
class IPostProcessStage
{
public:
	virtual void		Initialize() {};
	virtual TextureID	RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture) = 0; //Assume input texture size is always full size of the window
	void				SetEnabled(bool enabled);
	bool				Enabled = true;
private:

};


struct PostSceneTextures
{
	PostProcessRenderTarget HalfResTexture; 
	PostProcessRenderTarget QuarterResTexture;
	PostProcessRenderTarget HalfQuarterTexture; // 1/8th Resolution
	ScreenSize				HalfResSize;
	ScreenSize				QuarterResSize;
	ScreenSize				HalfQuarterSize;
};

class PostProcess
{
public:
	void				Intitialize();
	void				GenerateLowResTextures();
	PostSceneTextures	GetPostSceneTextures();
	void				RegisterPostProcess(std::string_view postProcessString, IPostProcessStage* stage);
	void				SetEnabled(std::string_view postProcess, bool enabled);
	PostSceneTextures	PostTextures;
private:
	void				RenderToTexture(ID3D12GraphicsCommandList* commandList, PostProcessRenderTarget target, ScreenSize screenSize, Renderer* renderer);
	std::unordered_map<std::string_view, IPostProcessStage*> postProcessStages;
};

extern PostProcess GPostProcess;