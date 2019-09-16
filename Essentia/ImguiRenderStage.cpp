#include "pch.h"
#include "ImguiRenderStage.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "Renderer.h"
#include "Entity.h"
#include "PostProcess.h"
#include "PostProcessDepthOfFieldStage.h"

void ImguiRenderStage::Initialize()
{
	auto window = EngineContext::Context->RendererInstance->GetWindow();
	auto device = EngineContext::Context->DeviceResources->GetDevice();

	imguiHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true);
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window->GetWindowHandle());
	ImGui_ImplDX12_Init(device, CFrameBufferCount,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		imguiHeap.hCPUHeapStart,
		imguiHeap.hGPUHeapStart);
}

void ImguiRenderStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto em = EngineContext::Context->EntityManager;
	auto commandList = EngineContext::Context->RendererInstance->GetDefaultCommandList();
	uint32_t dirLightCount;
	uint32_t pointLightCount;
	auto dirLights = em->GetComponents<DirectionalLightComponent>(dirLightCount);
	auto pointLights = em->GetComponents<PointLightComponent>(pointLightCount);
	auto pointLightEntities = em->GetEntities<PointLightComponent>(pointLightCount);
	auto transform = em->GetTransform(pointLightEntities[0]);

	ID3D12DescriptorHeap* heaps[] = { imguiHeap.pDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, heaps);
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	/*if (show)
		ImGui::ShowDemoWindow(&show);*/

	{
		static float f = 0.0f;
		static int counter = 0;
		static bool vsync = false;
		ImGui::Begin("Essentia");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("Basic Editor");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Post Process Window", &show);					// Edit bools storing our window open/close state
		ImGui::Checkbox("Vsync", &vsync);
		ImGui::ColorEdit3("Dir Light Color", (float*)& dirLights[0].Color.x); // Edit 3 floats representing a color
		ImGui::DragFloat3("Dir Light Direction", (float*)& dirLights[0].Direction, 0.1f, -1.f, 1.f);
		ImGui::SliderFloat("Dir Light Intensity", (float*)& dirLights[0].Intensity, 0.0f, 100.f, "%.3f", 2.1f);
		ImGui::SliderFloat("Point Light Range", &pointLights[0].Range, 0.0f, 100.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::DragFloat3("Point Light Pos", (float*)& transform.Position->x, 0.1f);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		EngineContext::Context->RendererInstance->SetVSync(vsync);
	}

	if (show)
	{
		auto dofStage = (PostProcessDepthOfFieldStage*)(GPostProcess.GetPostProcessStage("DepthOfField"));
		static bool dof = true;
		ImGui::Begin("Post Process", &show);
		ImGui::Checkbox("Depth Of Field", &dof);
		ImGui::DragFloat("Focus Plane", &dofStage->DofParams.FocusPlaneZ);
		ImGui::DragFloat("Scale", &dofStage->DofParams.Scale, 0.01f, 0.f, 1.f, "%.3f",0.5f);
		GPostProcess.SetEnabled("DepthOfField", dof);
		ImGui::End();
	}


	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImguiRenderStage::CleanUp()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
