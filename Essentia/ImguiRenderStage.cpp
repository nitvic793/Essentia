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
#include "GameStateManager.h"

using namespace DirectX;

void DrawTree(EntityHandle entity, EntityHandle& selected)
{
	ImGuiTreeNodeFlags treeNodeFlags = 0;
	if (entity.ID == selected.ID)
		treeNodeFlags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
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
	std::vector<std::string> matList;
	std::string meshName = ""; // Needs to be declared here to maintain state
	std::string matName = "";
public:
	ImguiVisitor()
	{
		auto meshManager = GContext->MeshManager;
		meshList = meshManager->GetAllMeshNames();
		matList = GContext->ShaderResourceManager->GetAllMaterialNames();
	}

	virtual void Visit(const char* compName, const char* name, float& val) override
	{
		ImGui::PushID(compName);
		ImGui::DragFloat(name, &val, 0.05f);
		ImGui::PopID();
	}

	virtual void Visit(const char* compName, const char* name, uint32& val) override
	{
		int value = (int32)val;
		ImGui::PushID(compName);
		ImGui::DragInt(name, &value, 0.05f);
		ImGui::PopID();
		val = value;
	}

	virtual void Visit(const char* compName, const char* name, std::string_view& val) override
	{
		ImGui::PushID(compName);
		ImGui::Text("%s: %s",name, val.data());
		ImGui::PopID();
	}

	virtual void Visit(const char* compName, const char* name, int32& val) override
	{
		int value = (int32)val;
		ImGui::PushID(compName);
		ImGui::DragInt(name, &value, 0.05f);
		ImGui::PopID();
		val = value;
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
		matName = GContext->ShaderResourceManager->GetMaterialName(val);

		ImGui::PushID(compName);
		if (ImGui::BeginCombo("##matCombo", matName.c_str()))
		{
			for (auto mat : matList)
			{
				bool isSelected = (strcmp(matName.c_str(), mat.c_str()) == 0);
				if (ImGui::Selectable(mat.c_str(), isSelected))
				{
					matName = mat;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		val = GContext->ShaderResourceManager->GetMaterialHandle(matName.c_str());
		ImGui::PopID();
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

		if (!GContext->GameStateManager->IsPlaying())
		{
			if (ImGui::Button("Play"))
			{
				GContext->GameStateManager->SetIsPlaying(true);
			}
		}
		else
		{
			if (ImGui::Button("Pause"))
			{
				GContext->GameStateManager->SetIsPlaying(false);
			}
		}

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
			if (currentItem != nullptr)
				cm->AddComponent(currentItem, selectedEntities[0]);
		}

		ImGui::End();
	}




	ImGuizmo::SetRect(0.f, 0.f, camera.Width, camera.Height);

	ImGuizmo::Enable(true);
	static auto camProj = camera.GetProjection();
	static auto camView = camera.GetView();

	camProj = camera.GetProjection();
	camView = camera.GetView();

	ImGui::Begin("Manipulate");
	//ImGuizmo::DrawGrid(&camView.m[0][0], &camProj.m[0][0], identityMatrix, 100.f);
	//ImGuizmo::DrawCubes(&camView.m[0][0], &camProj.m[0][0], &matrix.m[0][0], 1);
	static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE currentGizmoMode(ImGuizmo::WORLD);
	if (ImGui::IsKeyPressed(90))
		currentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(69))
		currentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(82)) // r Key
		currentGizmoOperation = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Translate", currentGizmoOperation == ImGuizmo::TRANSLATE))
		currentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", currentGizmoOperation == ImGuizmo::ROTATE))
		currentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", currentGizmoOperation == ImGuizmo::SCALE))
		currentGizmoOperation = ImGuizmo::SCALE;

	if (currentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", currentGizmoMode == ImGuizmo::LOCAL))
			currentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", currentGizmoMode == ImGuizmo::WORLD))
			currentGizmoMode = ImGuizmo::WORLD;
	}

	static bool useSnap(false);
	if (ImGui::IsKeyPressed(83))
		useSnap = !useSnap;
	ImGui::Checkbox("", &useSnap);
	ImGui::SameLine();
	static XMFLOAT3 snapConfig(1.f, 1.f, 1.f);
	XMFLOAT3 snap;
	switch (currentGizmoOperation)
	{
	case ImGuizmo::TRANSLATE:
		snap = snapConfig;
		ImGui::InputFloat3("Snap", &snap.x);
		break;
	case ImGuizmo::ROTATE:
		snap = snapConfig;
		ImGui::InputFloat("Angle Snap", &snap.x);
		break;
	case ImGuizmo::SCALE:
		snap = snapConfig;
		ImGui::InputFloat("Scale Snap", &snap.x);
		break;
	}

	ImGui::End();

	EntityHandle* entities = em->GetEntities<SelectedComponent>(count);
	auto transform = em->GetTransform(entities[0]);
	auto s = XMMatrixScalingFromVector(XMLoadFloat3(transform.Scale));
	auto r = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(transform.Rotation));
	auto t = XMMatrixTranslationFromVector(XMLoadFloat3(transform.Position));

	auto srt = s * r * t;
	auto parent = em->GetParent(entities[0]);
	XMFLOAT4X4 matrix = em->GetWorldMatrix(entities[0]);
	XMStoreFloat4x4(&matrix, srt);

	ImGuizmo::Manipulate(&camView.m[0][0], &camProj.m[0][0], currentGizmoOperation, currentGizmoMode, &matrix.m[0][0], NULL, useSnap ? &snap.x : NULL);

	//if (em->HasValidParent(entities[0])) {
	//	auto parentMatrix = em->GetWorldMatrix(entities[0]);
	//	auto p = XMLoadFloat4x4(&parentMatrix);
	//	auto pInv = XMMatrixInverse(nullptr, p);
	//	auto mat = XMLoadFloat4x4(&matrix);
	//	mat = pInv * mat;
	//	XMStoreFloat4x4(&matrix, mat);
	//}

	XMVECTOR scale;
	XMVECTOR rotation;
	XMVECTOR translation;
	XMMatrixDecompose(&scale, &rotation, &translation, XMLoadFloat4x4(&matrix));
	XMFLOAT4 F;
	XMStoreFloat3(transform.Position, translation);
	XMStoreFloat3(transform.Scale, scale);
	XMStoreFloat4(&F, rotation);

	//transform.Rotation->x = atan2(2.0 * (F.x * F.y + F.z * F.w), 1 - 2 * (F.y * F.y + F.z * F.z)); //roll
	//transform.Rotation->y = asin(2.0 * (F.x * F.z - F.w * F.y)); //pitch
	//transform.Rotation->z = atan2(2.0 * (F.x * F.w + F.y * F.z), 1 - 2 * (F.y * F.y + F.z * F.z)); //yaw

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImguiRenderStage::CleanUp()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
