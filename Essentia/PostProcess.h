#pragma once
#include "Declarations.h"
#include "EngineContext.h"
#include <unordered_map>
#include "FrameContext.h"

struct BlurParams
{
	DirectX::XMFLOAT2	Direction; // 0-Horizontal, 1-Vertical
	float				Width;
	float				Height;
};

//TODO : Pass the processed screen texture to the next stage
class IPostProcessStage
{
public:
	virtual void		Initialize() {};
	virtual TextureID	RenderPostProcess(uint32 backbufferIndex, TextureID inputTexture, const FrameContext& frameContext) = 0; //Assume input texture size is always full size of the window
	void				SetEnabled(bool enabled);
	bool				Enabled = true;
protected:
	void RenderToSceneTarget(TextureID inputTexture);
private:
};


//Stores scene texture in half, quarter and 1/8th resolutions
struct PostSceneTextures
{
	SceneRenderTarget	HalfResTexture; 
	SceneRenderTarget	QuarterResTexture;
	SceneRenderTarget	HalfQuarterTexture; // 1/8th Resolution
	ScreenSize			HalfResSize;
	ScreenSize			QuarterResSize;
	ScreenSize			HalfQuarterSize;
};

using PostProcessMap = std::unordered_map<std::string_view, IPostProcessStage*>;

class PostProcess
{
public:
	void				Intitialize();
	void				GenerateLowResTextures();
	PostSceneTextures	GetPostSceneTextures();
	//Blurs given texture into final given target.
	void				RenderBlurTexture(TextureID input, 
										  ScreenSize screenSize,
										  uint32 backBufferIndex, 
										  SceneRenderTarget target, 
										  SceneRenderTarget intermediateTarget, 
										  ConstantBufferView vertCBV, 
										  ConstantBufferView horCBV);
	void				RegisterPostProcess(std::string_view postProcessString, IPostProcessStage* stage);
	void				SetEnabled(std::string_view postProcess, bool enabled);
	IPostProcessStage*	GetPostProcessStage(std::string_view name);
	PostSceneTextures	PostTextures;
	const PostProcessMap& GetStagesMap();
private:
	void				RenderToTexture(ID3D12GraphicsCommandList* commandList, SceneRenderTarget target, ScreenSize screenSize, Renderer* renderer);
	PostProcessMap		postProcessStages;
};

extern PostProcess GPostProcess;