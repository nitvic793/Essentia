#include "pch.h"
#include "Animation.h"

using namespace DirectX;

uint32_t FindPosition(float AnimationTime, const AnimationChannel* channel)
{
	const auto keySize = channel->PositionKeys.size();
	for (uint32_t i = 0; i < keySize - 1; i++) {
		if (AnimationTime < (float)channel->PositionKeys.at(i + 1).Time) {
			return i;
		}
	}

	assert(0);
	return 0;
}

uint32_t FindScaling(float AnimationTime, const AnimationChannel* channel)
{
	const auto size = channel->ScalingKeys.size();
	for (uint32_t i = 0; i < size - 1; i++) {
		if (AnimationTime < (float)channel->ScalingKeys.at(i + 1).Time) {
			return i;
		}
	}

	assert(0);
	return 0;
}

uint32_t FindRotation(float AnimationTime, const AnimationChannel* channel)
{
	const auto size = channel->RotationKeys.size();
	for (uint32_t i = 0; i < size - 1; i++) {
		if (AnimationTime < (float)channel->RotationKeys.at(i + 1).Time) {
			return i;
		}
	}

	assert(0);
	return 0;
}

XMVECTOR InterpolatePosition(float animTime, const AnimationChannel* channel)
{
	auto outFloat3 = XMFLOAT3();
	auto Out = XMVectorSet(0, 0, 0, 0);
	if (channel->PositionKeys.size() == 1) {
		Out = XMLoadFloat3(&channel->PositionKeys[0].Value);
		return Out;
	}

	uint32_t PositionIndex = FindPosition(animTime, channel);
	uint32_t NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < channel->PositionKeys.size());

	float DeltaTime = (float)(channel->PositionKeys[NextPositionIndex].Time - channel->PositionKeys[PositionIndex].Time);
	float Factor = (animTime - (float)channel->PositionKeys[PositionIndex].Time) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);

	auto Start = XMLoadFloat3(&channel->PositionKeys[PositionIndex].Value);
	auto End = XMLoadFloat3(&channel->PositionKeys[NextPositionIndex].Value);
	auto Delta = End - Start;
	Out = Start + Factor * Delta;
	return Out;
}


XMVECTOR InterpolateScaling(float animTime, const AnimationChannel* channel)
{
	auto outFloat3 = XMFLOAT3();
	auto Out = XMVectorSet(0, 0, 0, 0);
	if (channel->ScalingKeys.size() == 1) {
		Out = XMLoadFloat3(&channel->ScalingKeys[0].Value);
		return Out;
	}

	uint32_t ScaleIndex = FindScaling(animTime, channel);
	uint32_t NextScaleIndex = (ScaleIndex + 1);
	assert(NextScaleIndex < channel->ScalingKeys.size());

	float DeltaTime = (float)(channel->ScalingKeys[NextScaleIndex].Time - channel->ScalingKeys[ScaleIndex].Time);
	float Factor = (animTime - (float)channel->ScalingKeys[ScaleIndex].Time) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);

	auto Start = XMLoadFloat3(&channel->ScalingKeys[ScaleIndex].Value);
	auto End = XMLoadFloat3(&channel->ScalingKeys[NextScaleIndex].Value);
	auto Delta = End - Start;
	Out = Start + Factor * Delta;
	//XMStoreFloat3(&outFloat3, Out);
	return Out;
}

XMFLOAT4 InterpolateRotation(float animTime, const AnimationChannel* channel)
{
	auto outFloat4 = XMFLOAT4();
	auto Out = XMVectorSet(0, 0, 0, 0);
	if (channel->RotationKeys.size() == 1) {
		//Out = XMLoadFloat4(&channel->RotationKeys[0].Value);
		return channel->RotationKeys[0].Value;
	}

	uint32_t RotationIndex = FindRotation(animTime, channel);
	uint32_t NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < channel->RotationKeys.size());

	float DeltaTime = (float)(channel->RotationKeys[NextRotationIndex].Time - channel->RotationKeys[RotationIndex].Time);
	float Factor = (animTime - (float)channel->RotationKeys[RotationIndex].Time) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);

	auto StartRotationQ = XMLoadFloat4(&channel->RotationKeys[RotationIndex].Value);
	auto EndRotationQ = XMLoadFloat4(&channel->RotationKeys[NextRotationIndex].Value);
	Out = XMQuaternionSlerp((StartRotationQ), (EndRotationQ), Factor);
	Out = XMQuaternionNormalize(Out);
	XMStoreFloat4(&outFloat4, Out);
	return outFloat4;
}