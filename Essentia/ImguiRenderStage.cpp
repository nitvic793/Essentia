#include "pch.h"
#include "ImguiRenderStage.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "Renderer.h"
#include "Entity.h"
#include "PostProcess.h"
#include "PostProcessDepthOfFieldStage.h"
#include "Interface.h"
#include "ComponentReflector.h"

using namespace DirectX;

class ImguiVisitor : public IVisitor
{
	std::vector<std::string> meshList; 
	std::string meshName = ""; // Needs to be declared here to maintain state
public:
	virtual void Visit(const char* compName, const char* name, float& val) override
	{
		ImGui::PushID(compName);
		ImGui::DragFloat(name, &val, 0.05f);
		ImGui::PopID();
	}

	virtual void Visit(const char* compName, const char* name, XMFLOAT3& val) override
	{
		ImGui::PushID(compName);
		auto label = std::string(name);
		if (std::string(name).find("Color") != std::string::npos)
		{
			ImGui::ColorEdit3(label.c_str(), &val.x);
		}
		else
		{
			ImGui::DragFloat3(label.c_str(), &val.x, 0.05f);
		}
		ImGui::PopID();
	}

	virtual void Visit(const char* compName, const char* name, MeshHandle& val) override
	{
		auto meshManager = GContext->MeshManager;
		meshName = meshManager->GetName(val);
		meshList = meshManager->GetAllMeshNames();

		ImGui::PushID(compName);
		if (ImGui::BeginCombo("##meshCombo", meshName.c_str(), ImGuiSelectableFlags_SpanAllColumns))
		{
			for (auto mesh : meshList)
			{
				bool isSelected = (strcmp(meshName.c_str(), mesh.c_str()) == 0);
				if (ImGui::Selectable(mesh.c_str(), isSelected))
				{
					meshName = mesh;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		val = meshManager->GetMeshHandle(meshName.c_str());
		ImGui::PopID();
	}

	virtual void Visit(const char* compName, const char* name, MaterialHandle& val) override
	{

	}
};

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
	GRenderStageManager.RegisterStage("ImguiRenderStage", this);
}

void ImguiRenderStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto em = EngineContext::Context->EntityManager;
	auto cm = GContext->EntityManager->GetComponentManager();
	auto commandList = EngineContext::Context->RendererInstance->GetDefaultCommandList();

	ID3D12DescriptorHeap* heaps[] = { imguiHeap.pDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, heaps);
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	{
		static float f = 0.0f;
		static int counter = 0;
		static bool vsync = false;
		ImGui::Begin("Essentia");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Checkbox("Vsync", &vsync);

		if (ImGui::CollapsingHeader("Render Stages"))
		{
			auto stages = GRenderStageManager.GetRenderStageMap();
			for (auto stage : stages)
			{
				ImGui::Checkbox(stage.first.data(), &stage.second->Enabled);
			}
		}

		if (ImGui::CollapsingHeader("PostProcess Stages"))
		{
			auto dofStage = (PostProcessDepthOfFieldStage*)(GPostProcess.GetPostProcessStage("DepthOfField"));
			static bool dof = true;

			ImGui::DragFloat("DOF Focus Plane", &dofStage->DofParams.FocusPlaneZ);
			ImGui::DragFloat("DOF Scale", &dofStage->DofParams.Scale, 0.01f, 0.f, 1.f, "%.3f", 0.5f);

			auto postProcesssMap = GPostProcess.GetStagesMap();
			for (auto stage : postProcesssMap)
			{
				ImGui::Checkbox(stage.first.data(), &stage.second->Enabled);
			}
		}

		if (ImGui::CollapsingHeader("Entities"))
		{
			static int selected = -1;
			uint32 count = 0;
			auto entities = em->GetEntities<PositionComponent>(count);
			for (uint32 i = 0; i < count; ++i)
			{
				if (ImGui::Selectable(std::to_string(i).c_str(), selected == i, ImGuiSelectableFlags_AllowDoubleClick))
				{
					selected = i;
					uint32 selectedEntityCount = 0;
					auto selectedEntities = em->GetEntities<SelectedComponent>(selectedEntityCount);
					if (selectedEntityCount > 0)
					{
						for (uint32 i = 0; i < selectedEntityCount; ++i)
						{
							em->GetComponentManager()->RemoveComponent<SelectedComponent>(selectedEntities[i]);
						}
					}
					em->AddComponent<SelectedComponent>(entities[i]);
				}
			}
		}

		ImGui::End();
		GContext->RendererInstance->SetVSync(vsync);
	}

	uint32 count = 0;
	auto selectedEntities = em->GetEntities<SelectedComponent>(count);
	static bool showEntity = false;
	if (count > 0)
	{
		static ImguiVisitor visitor;
		ImGui::Begin("Entity", &showEntity);
		auto components = em->GetComponents(selectedEntities[0]);
		for (auto comp : components)
		{
			if (ImGui::CollapsingHeader(comp.ComponentName.data(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				GComponentReflector.VisitFields(comp.ComponentName.data(), comp.Data, &visitor);
				ImGui::PushID(comp.ComponentName.data());
				if (ImGui::Button("Remove"))
				{
					cm->RemoveComponent(comp.ComponentName.data(), selectedEntities[0]);
				}
				ImGui::PopID();
			}
		}

		ImGui::NewLine();

		static int selected = -1;
		Vector<const char*> list = cm->GetComponentNameList();
		static const char* currentItem = NULL;

		if (ImGui::BeginCombo("##combo", currentItem))
		{
			for (auto component : list)
			{
				bool isSelected = (currentItem == component);
				if (ImGui::Selectable(component, isSelected))
					currentItem = component;
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();
		if (ImGui::Button("Add Component"))
		{
			cm->AddComponent(currentItem, selectedEntities[0]);
		}

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
