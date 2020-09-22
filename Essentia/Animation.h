#pragma once

#include <DirectXMath.h>
#include <unordered_map>
#include "OGLMath.h"

struct VectorKey
{
	double				Time;
	DirectX::XMFLOAT3	Value;
};

struct QuaternionKey
{
	double				Time;
	DirectX::XMFLOAT4	Value;
};

struct AnimationChannel
{
	std::string					NodeName;
	std::vector<VectorKey>		PositionKeys;
	std::vector<QuaternionKey>	RotationKeys;
	std::vector<VectorKey>		ScalingKeys;
};

struct Animation
{
	std::string AnimationName;
	double TicksPerSecond;
	double Duration;
	std::vector<AnimationChannel> Channels;
	std::unordered_map<std::string, uint32_t> NodeChannelMap;
};

struct MeshAnimationDescriptor
{
	std::string RootNode;
	DirectX::XMFLOAT4X4 GlobalInverseTransform;
	std::unordered_map<std::string, std::vector<std::string>> NodeHeirarchy;
	std::unordered_map<std::string, DirectX::XMFLOAT4X4> NodeTransformsMap;
	std::unordered_map<std::string, uint32_t> AnimationIndexMap;
	std::vector<Animation> Animations;

	Animation* GetAnimation(std::string animName)
	{
		if (AnimationIndexMap.find(animName) == AnimationIndexMap.end())
		{
			return nullptr;
		}

		return &Animations[AnimationIndexMap[animName]];
	}

	const Animation& GetAnimation(uint32_t index) const
	{
		return Animations[index];
	}

	const std::string& GetAnimationName(uint32_t index) const
	{
		return Animations[index].AnimationName;
	}

	int32_t GetChannelIndex(uint32_t animationIndex, std::string node) const
	{
		auto& map = Animations[animationIndex].NodeChannelMap;
		auto index = -1;
		while (map.find(node) == map.end())
		{
			return index;
		}
		
		index = map.find(node)->second;
		return index;
	}

	const AnimationChannel* GetChannel(uint32_t animIndex, std::string node) const
	{
		auto channelIndex = GetChannelIndex(animIndex, node);
		if (channelIndex == -1)
		{
			return nullptr;
		}

		return &Animations[animIndex].Channels[channelIndex];
	}

	std::vector<std::string> GetAnimationNames() const
	{
		std::vector<std::string> animNames;
		for (auto anim : AnimationIndexMap) 
		{
			animNames.push_back(anim.first);
		}
		
		return animNames;
	}

	uint32_t GetAnimationCount() const
	{
		return (uint32_t)Animations.size();
	}

	bool IsAnimationIndexValid(uint32_t animationIndex) const
	{
		return (animationIndex <= Animations.size() - 1 && animationIndex >= 0);
	}

};

uint32_t FindPosition(float AnimationTime, const AnimationChannel* channel);
uint32_t FindScaling(float AnimationTime, const AnimationChannel* channel);
uint32_t FindRotation(float AnimationTime, const AnimationChannel* channel);

DirectX::XMFLOAT3 InterpolatePosition(float animTime, const AnimationChannel* channel);
DirectX::XMFLOAT3 InterpolateScaling(float animTime, const AnimationChannel* channel);
DirectX::XMFLOAT4 InterpolateRotation(float animTime, const AnimationChannel* channel);