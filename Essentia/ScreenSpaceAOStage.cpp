#include "pch.h"
#include "ScreenSpaceAOStage.h"
#include "RenderTargetManager.h"
#include "Renderer.h"
#include "PipelineStates.h"
#include "SceneResources.h"
#include <DirectXPackedVector.h>
#include "d3dx12.h"

using namespace DirectX;

XMFLOAT4X4 GetProjTex(const XMFLOAT4X4& projection)
{
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	auto P = XMLoadFloat4x4(&projection);
	XMFLOAT4X4 projTex;
	XMStoreFloat4x4(&projTex, XMMatrixTranspose(P * T));
	return projTex;
}

void ScreenSpaceAOStage::Initialize()
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto renderer = GContext->RendererInstance;
	auto sz = renderer->GetScreenSize();
	auto format = DXGI_FORMAT_R32_FLOAT; //For debugging, should be R32_FLOAT;
	aoRenderTarget = CreateSceneRenderTarget(GContext, sz.Width, sz.Height, format);
	aoBlurIntermediate = CreateSceneRenderTarget(GContext, sz.Width, sz.Height, format);
	aoBlurFinal = CreateSceneRenderTarget(GContext, sz.Width, sz.Height, format);
	aoParamsCBV = shaderResourceManager->CreateCBV(sizeof(AOParams));
	aoVSParamsCBV = shaderResourceManager->CreateCBV(sizeof(AOVSParams));

	aoBlurDir1 = shaderResourceManager->CreateCBV(sizeof(SSAOBlurParams));
	aoBlurDir2 = shaderResourceManager->CreateCBV(sizeof(SSAOBlurParams));

	BuildOffsetVectors(); 
	memcpy(aoParams.OffsetVectors, mOffsets, sizeof(XMFLOAT4) * 14);

	Vector<float> weights = CalcGaussWeights(2.5f);
	aoParams.BlurWeights[0] = XMFLOAT4(&weights.GetData()[0]);
	aoParams.BlurWeights[1] = XMFLOAT4(&weights.GetData()[4]);
	aoParams.BlurWeights[2] = XMFLOAT4(&weights.GetData()[8]);

	aoParams.OcclusionRadius = 0.5f;
	aoParams.OcclusionFadeStart = 0.2f;
	aoParams.OcclusionFadeEnd = 1.0f;
	aoParams.SurfaceEpsilon = 0.05f;

	BuildRandomVectorTexture(renderer->GetDefaultCommandList());
	GSceneResources.AmbientOcclusion = aoBlurFinal;
	GRenderStageManager.RegisterStage("ScreenSpaceAOStage", this);
}

void ScreenSpaceAOStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto rtManager = GContext->RenderTargetManager;
	auto sz = renderer->GetScreenSize();

	auto camera = frameContext.Camera;
	auto projTransposed = camera->GetProjectionTransposed();
	aoParams.Projection = projTransposed;
	aoParams.InvProjection = camera->GetInverseProjectionTransposed();
	aoParams.ProjectionTex = GetProjTex(camera->GetProjection());
	aoParams.ScreenSize = XMFLOAT2((float)sz.Width, (float)sz.Height);

	AOVSParams vsParams = { aoParams.InvProjection };

	shaderResourceManager->CopyToCB(frameIndex, { &vsParams, sizeof(vsParams) }, aoVSParamsCBV);
	shaderResourceManager->CopyToCB(frameIndex, { &aoParams, sizeof(aoParams) }, aoParamsCBV);
	renderer->TransitionBarrier(commandList, aoRenderTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	renderer->SetTargetSize(commandList, sz);
	renderer->SetRenderTargets(&aoRenderTarget.RenderTarget, 1, nullptr);
	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(aoRenderTarget.RenderTarget), ColorValues::ClearColor, 0, nullptr);
	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.ScreenSpaceAOPSO));

	TextureID textures[] = { GSceneResources.DepthPrePass.Texture, randomVecTextureId };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));

	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, aoParamsCBV);
	renderer->SetConstantBufferView(commandList, RootSigCBVertex0, aoVSParamsCBV);

	//Draw Screen Quad
	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(6, 1, 0, 0);

	renderer->TransitionBarrier(commandList, aoRenderTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	BlurSSAO(frameIndex);
}

void ScreenSpaceAOStage::BlurSSAO(uint32 frameIndex)
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto rtManager = GContext->RenderTargetManager;
	auto sz = renderer->GetScreenSize();

	SSAOBlurParams blurDirHorizontal = { XMFLOAT2(1.f / (float)sz.Width, 0.f) };
	SSAOBlurParams blurDirVertical = { XMFLOAT2(0.f, 1.f / (float)sz.Height) };

	shaderResourceManager->CopyToCB(frameIndex, { &blurDirHorizontal, sizeof(blurDirHorizontal) }, aoBlurDir1);
	shaderResourceManager->CopyToCB(frameIndex, { &blurDirVertical, sizeof(blurDirVertical) }, aoBlurDir2);

	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.SSAOBlurPSO));

	renderer->TransitionBarrier(commandList, aoBlurIntermediate.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	renderer->SetRenderTargets(&aoBlurIntermediate.RenderTarget, 1, nullptr);
	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(aoBlurIntermediate.RenderTarget), ColorValues::ClearColor, 0, nullptr);

	TextureID textures[] = { GSceneResources.DepthPrePass.Texture, randomVecTextureId, aoRenderTarget.Texture };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBAll1, aoBlurDir1);
	//Draw Screen Quad
	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(6, 1, 0, 0);

	renderer->TransitionBarrier(commandList, aoBlurIntermediate.Resource,  D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	renderer->TransitionBarrier(commandList, aoBlurFinal.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	renderer->SetRenderTargets(&aoBlurFinal.RenderTarget, 1, nullptr);
	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(aoBlurFinal.RenderTarget), ColorValues::ClearColor, 0, nullptr);
	renderer->SetConstantBufferView(commandList, RootSigCBAll1, aoBlurDir2);
	TextureID textures2[] = { GSceneResources.DepthPrePass.Texture, randomVecTextureId, aoBlurIntermediate.Texture };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures2, _countof(textures2));

	//Draw Screen Quad
	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(6, 1, 0, 0);

	renderer->TransitionBarrier(commandList, aoBlurFinal.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

}

void ScreenSpaceAOStage::BuildRandomVectorTexture(ID3D12GraphicsCommandList* commandList)
{
	auto resourceManager = GContext->ResourceManager;
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto device = GContext->DeviceResources->GetDevice();
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	randomVectorResourceId = resourceManager->CreateResource(texDesc, nullptr, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	auto randomVectorMap = resourceManager->GetResource(randomVectorResourceId);

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(randomVectorMap, 0, num2DSubresources);

	auto uploadBufferId = resourceManager->CreateResource(CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), nullptr, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_FLAG_NONE, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD));
	auto mRandomVectorMapUploadBuffer = resourceManager->GetResource(uploadBufferId);

	Vector<PackedVector::XMCOLOR> initData;
	initData.SetSize(256 * 256);
	//PackedVector::XMCOLOR initData[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			// Random vector in [0,1].  We will decompress in shader to [-1,1].
			XMFLOAT3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());

			initData[(uint32)i * 256 + j] = PackedVector::XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData.GetData();
	subResourceData.RowPitch = 256 * sizeof(PackedVector::XMCOLOR);
	subResourceData.SlicePitch = subResourceData.RowPitch * 256;

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(randomVectorMap,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(commandList, randomVectorMap, mRandomVectorMapUploadBuffer,
		0, 0, num2DSubresources, &subResourceData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(randomVectorMap,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	randomVecTextureId = shaderResourceManager->CreateTexture(randomVectorMap);
}

void ScreenSpaceAOStage::BuildOffsetVectors()
{
	mOffsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	mOffsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	mOffsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	mOffsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	mOffsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	mOffsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	mOffsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	mOffsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = MathHelper::RandF(0.25f, 1.0f);

		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&mOffsets[i]));

		XMStoreFloat4(&mOffsets[i], v);
	}
}

Vector<float> ScreenSpaceAOStage::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;
	int blurRadius = (int)ceil(2.0f * sigma);

	//assert(blurRadius <= MaxBlurRadius);
	Vector<float> weights;
	weights.SetSize(2 * blurRadius + 1, Mem::GetFrameAllocator());

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}

	return weights;
}
