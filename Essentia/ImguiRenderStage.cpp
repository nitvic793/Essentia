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
#include "ImGuizmo.h"

using namespace DirectX; 

void DrawTree(EntityHandle entity, EntityHandle& selected)
{
	ImGuiTreeNodeFlags treeNodeFlags = 0;
	auto children = GContext->EntityManager->GetChildren(entity);
	if (children.size() == 0)
	{
		treeNodeFlags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_NoTreePushOnOpen;
		ImGui::TreeNodeEx(GContext->EntityManager->GetEntityName(entity).data(), treeNodeFlags);
		if (ImGui::IsItemClicked())
			selected = entity;
	}
	else if (ImGui::TreeNode(GContext->EntityManager->GetEntityName(entity).data()))
	{
		if (ImGui::IsItemClicked())
			selected = entity;
		for (auto child : children)
		{
			DrawTree(child, selected);
		}

		ImGui::TreePop();
	}
}

class ImguiVisitor : public IVisitor
{
	std::vector<std::string> meshList;
	std::string meshName = ""; // Needs to be declared here to maintain state
public:
	ImguiVisitor()
	{
		auto meshManager = GContext->MeshManager;
		meshList = meshManager->GetAllMeshNames();
	}

	virtual void Visit(const char* compName, const char* name, float& val) override
	{
		ImGui::PushID(compName);
		ImGui::DragFloat(name, &val, 0.05f);
		ImGui::PopID();
	}

	virtual void Visit(const char* compName, const char* name, bool& val) override
	{
		ImGui::PushID(compName);
		ImGui::Checkbox(name, &val);
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

		ImGui::PushID(compName);
		if (ImGui::BeginCombo("##meshCombo", meshName.c_str()))
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
	uint32 count = 0;
	auto em = EngineContext::Context->EntityManager;
	auto cm = GContext->EntityManager->GetComponentManager();
	auto commandList = EngineContext::Context->RendererInstance->GetDefaultCommandList();
	const auto& camera = em->GetComponents<CameraComponent>(count)[0].CameraInstance;

	ID3D12DescriptorHeap* heaps[] = { imguiHeap.pDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, heaps);
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
	ImGuizmo::SetOrthographic(camera.IsOrthographic);
	ImGuizmo::BeginFrame();

	{
		static float f = 0.0f;
		static int counter = 0;
		static bool vsync = false;
		ImGui::Begin("Essentia");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Checkbox("Vsync", &vsync);

		//if (!GContext->IsPlaying())
		//{
		//	if (ImGui::Button("Play"))
		//	{
		//		GContext->bIsPlaying = true;
		//	}
		//}
		//else
		//{
		//	if (ImGui::Button("Pause"))
		//	{
		//		GContext->bIsPlaying = false;
		//	}
		//}

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
				auto children = em->GetChildren(entities[i]);
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

		if (ImGui::CollapsingHeader("Entity Tree"))
		{
			uint32 count = 0;
			auto entities = em->GetEntities<PositionComponent>(count);
			static EntityHandle selected = { Handle{CRootParentEntityIndex} };
			for (uint32 i = 0; i < count; ++i)
			{
				if (!em->HasValidParent(entities[i]))
					DrawTree(entities[i], selected);
			}

			if (selected.Handle.Index != CRootParentEntityIndex)
			{
				uint32 selectCount;
				auto selectedEntities = em->GetEntities<SelectedComponent>(selectCount);
				if (selectCount > 0)
				{
					for (uint32 i = 0; i < selectCount; ++i)
					{
						em->GetComponentManager()->RemoveComponent<SelectedComponent>(selectedEntities[i]);
					}
				}
				em->AddComponent<SelectedComponent>(selected);
			}
		}

		ImGui::End();
		GContext->RendererInstance->SetVSync(vsync);
	}

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

	EntityHandle* entities = em->GetEntities<SelectedComponent>(count);
	auto transform = em->GetTransform(entities[0]);
	auto s = XMMatrixScalingFromVector(XMLoadFloat3(transform.Scale));
	auto r = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(transform.Rotation));
	auto t = XMMatrixTranslationFromVector(XMLoadFloat3(transform.Position));

	auto srt = s * r * t;
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, srt);

	static const float identityMatrix[16] =
	{ 1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	ImGuizmo::SetRect(0.f, 0.f, camera.Width, camera.Height);

	ImGuizmo::Enable(true);
	static auto camProj = camera.GetProjection();
	static auto camView = camera.GetView();

	camProj = camera.GetProjection();
	camView = camera.GetView();
	
	//ImGuizmo::DrawGrid(&camView.m[0][0], &camProj.m[0][0], identityMatrix, 100.f);
	//ImGuizmo::DrawCubes(&camView.m[0][0], &camProj.m[0][0], &matrix.m[0][0], 1);


	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImguiRenderStage::CleanUp()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
