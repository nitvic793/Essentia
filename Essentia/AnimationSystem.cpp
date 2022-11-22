#include "pch.h"
#include "AnimationSystem.h"
#include "Trace.h"

using namespace DirectX;

void AnimationSystem::Initialize()
{
}

void AnimationSystem::Update(float deltaTime, float totalTime)
{
	uint32 count = 0;
	EntityManager* entityManager = GContext->EntityManager;
	// Consolidate animation component - add/remove animation component depending on mesh
	Vector<EntityHandle> entities = entityManager->GetComponentManager()->GetEntities<DrawableComponent>();
	for (auto entity : entities)
	{
		DrawableComponent* component = entityManager->GetComponent<DrawableComponent>(entity);
		if (GContext->MeshManager->IsAnimated(component->Mesh))
		{
			AnimationComponent* animComp = entityManager->GetComponent<AnimationComponent>(entity);
			if (animComp == nullptr)
			{
				auto comp = AnimationComponent::Create(component->Mesh);
				entityManager->AddComponent(entity, comp);
			}
			component->Flags = component->Flags | kDrawableAnimatedMesh;
		}
		else
		{
			AnimationComponent* animComp = entityManager->GetComponentManager()->TryGetComponent<AnimationComponent>(entity);
			if (animComp != nullptr)
			{
				entityManager->GetComponentManager()->RemoveComponent<AnimationComponent>(entity);
			}
			component->Flags = (DrawableRenderFlags)(component->Flags & ~kDrawableAnimatedMesh);
		}
	}

	// Update animations
	AnimationComponent* animComponents = GContext->EntityManager->GetComponents<AnimationComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		if (!animComponents[i].IsPlaying)
		{
			GContext->ShaderResourceManager->CopyToCB(GContext->RendererInstance->GetCurrentBackbufferIndex(),
				{ &animComponents[i].ArmatureConstantBuffer, sizeof(PerArmatureConstantBuffer) },
				animComponents[i].ArmatureCBV);
			break;
		}
		const AnimationData& animData = GContext->MeshManager->GetAnimationData(animComponents[i].Mesh);
		if (!animData.Animations.IsAnimationIndexValid(animComponents[i].CurrentAnimationIndex))
		{
			es::Log("[error] Animation index: %d is invalid.", animComponents[i].CurrentAnimationIndex);
			continue;
		}

		animComponents[i].CurrentAnimation = animData.Animations.GetAnimationName(animComponents[i].CurrentAnimationIndex);
		const Animation& animation = animData.Animations.GetAnimation(animComponents[i].CurrentAnimationIndex);
		animComponents[i].TotalTime += deltaTime * animComponents[i].AnimationSpeed;
		BoneTransform(animComponents[i], animation);

		GContext->ShaderResourceManager->CopyToCB(GContext->RendererInstance->GetCurrentBackbufferIndex(),
			{ &animComponents[i].ArmatureConstantBuffer, sizeof(PerArmatureConstantBuffer) },
			animComponents[i].ArmatureCBV);
	}
}

void AnimationSystem::BoneTransform(AnimationComponent& animComponent, const Animation& animation)
{
	float totalTime = animComponent.TotalTime;
	float TicksPerSecond = (float)(animation.TicksPerSecond != 0 ? animation.TicksPerSecond : 25.0f);
	float TimeInTicks = totalTime * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)animation.Duration);
	ReadNodeHeirarchy(animComponent, animation, AnimationTime);

	for (uint32_t i = 0; i < animComponent.BoneInfoSize; i++)
	{
		XMFLOAT4X4 finalTransform;
		XMStoreFloat4x4(&finalTransform, XMMatrixTranspose(XMLoadFloat4x4(&animComponent.BoneInfoList[i].FinalTransform)));
		animComponent.ArmatureConstantBuffer.Bones[i] = finalTransform;
	}
}

void AnimationSystem::ReadNodeHeirarchy(AnimationComponent& animComponent, const Animation& animation, float animationTime)
{
	XMFLOAT4X4 identity;
	XMFLOAT4X4 globalFloat4x4;
	std::stack<std::string> nodeQueue;
	std::stack<XMFLOAT4X4> transformationQueue;

	uint32 animationIndex = animComponent.CurrentAnimationIndex;
	const AnimationData& animData = GContext->MeshManager->GetAnimationData(animComponent.Mesh);
	XMMATRIX globalInverse = XMLoadFloat4x4(&animData.Animations.GlobalInverseTransform);
	XMMATRIX rootTransform = XMMatrixIdentity();

	std::string rootNode = animData.Animations.RootNode;

	XMStoreFloat4x4(&identity, rootTransform);
	nodeQueue.push(rootNode);
	transformationQueue.push(identity);

	while (!nodeQueue.empty())
	{
		auto node = nodeQueue.top();
		auto parentTransformation = XMLoadFloat4x4(&transformationQueue.top());
		auto nodeTransformation = XMLoadFloat4x4(&animData.Animations.NodeTransformsMap.find(node)->second);

		nodeQueue.pop();
		transformationQueue.pop();

		auto anim = animData.Animations.GetChannel(animationIndex, node);
		if (anim != nullptr)
		{
			auto s = InterpolateScaling(animationTime, anim);
			auto scaling = XMMatrixScalingFromVector(s);

			auto r = InterpolateRotation(animationTime, anim);
			auto rotation = XMMatrixRotationQuaternion(XMVectorSet(r.y, r.z, r.w, r.x));
			auto rv = XMLoadFloat4(&r);

			auto t = InterpolatePosition(animationTime, anim);
			auto translation = XMMatrixTranslationFromVector(t);

			nodeTransformation = XMMatrixAffineTransformation(s, XMVectorSet(r.y, r.z, r.w, r.x), XMVectorSet(r.y, r.z, r.w, r.x), t);
			//nodeTransformation = XMMatrixAffineTransformation(s, rv, rv, t);
			//nodeTransformation += scaling * rotation * translation;
		}

		auto globalTransformation = nodeTransformation * parentTransformation;
		if (animData.BoneMapping.find(node) != animData.BoneMapping.end())
		{
			uint32_t BoneIndex = animData.BoneMapping.find(node)->second;
			auto finalTransform = XMMatrixTranspose(OGLtoXM(animComponent.BoneInfoList[BoneIndex].Offset)) * globalTransformation * globalInverse;
			XMStoreFloat4x4(&animComponent.BoneInfoList[BoneIndex].FinalTransform, finalTransform);
		}

		auto children = animData.Animations.NodeHeirarchy.find(node)->second;
		for (int i = (int)children.size() - 1; i >= 0; --i)
		{
			XMStoreFloat4x4(&globalFloat4x4, globalTransformation);
			nodeQueue.push(children[i]);
			transformationQueue.push(globalFloat4x4);
		}
	}
}
