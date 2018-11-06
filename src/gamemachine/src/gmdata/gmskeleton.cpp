#include "stdafx.h"
#include <gmskeleton.h>

namespace
{
	template <typename T>
	GMsize_t findIndex(GMDuration animationTime, const AlignedVector<GMSkeletonAnimationKeyframe<T>>& frames)
	{
		GM_ASSERT(frames.size() > 0);
		//for (auto& frame : frames)
		for (GMsize_t i = 0; i < frames.size() - 1; ++i)
		{
			const GMSkeletonAnimationKeyframe<T>& frame = frames[i + 1];
			if (animationTime < frame.time)
				return i;
		}

		GM_ASSERT(false);
		return 0;
	}
}

void GMSkeletalVertexBoneData::reset()
{
	D(d);
	GM_ZeroMemory(d->ids);
	GM_ZeroMemory(d->weights);
}

void GMSkeletalVertexBoneData::addData(GMsize_t boneId, GMfloat weight)
{
	D(d);
	for (GMsize_t i = 0; i < GM_array_size(d->ids); ++i)
	{
		if (d->weights[i] == 0.f)
		{
			d->ids[i] = boneId;
			d->weights[i] = weight;
			return;
		}
	}

	// 如果超过了BonesPerVertex骨骼，才会走到这里
	GM_ASSERT(false);
}

GMSkeletalAnimationEvaluator::GMSkeletalAnimationEvaluator(GMSkeletalNode* root, GMSkeleton* skeleton)
{
	D(d);
	setRootNode(root);
	setSkeleton(skeleton);
	d->globalInverseTransform = Inverse(root->getTransformToParent());
}

void GMSkeletalAnimationEvaluator::update(GMDuration dt)
{
	D(d);
	d->duration += dt;

	GMfloat ticks = d->duration * d->animation->frameRate;
	GMfloat animationTime = Fmod(ticks, d->animation->duration);

	updateNode(animationTime, d->rootNode, Identity<GMMat4>());
	AlignedVector<GMMat4>& transforms = d->transforms;

	GMsize_t boneCount = d->skeleton->getBones().getBones().size();
	transforms.resize(boneCount);

	for (GMsize_t i = 0; i < boneCount; ++i)
	{
		transforms[i] = d->skeleton->getBones().getBones()[i].finalTransformation;
	}
}

void GMSkeletalAnimationEvaluator::updateNode(GMfloat animationTime, GMSkeletalNode* node, const GMMat4& parentTransformation)
{
	D(d);
	const GMString& nodeName = node->getName();
	const GMMat4& transformation = node->getTransformToParent();
	const GMSkeletalAnimationNode* animationNode = findAnimationNode(nodeName);
	GMMat4 nodeTransformation = node->getTransformToParent();
	GMuint32 factor = 0;

	if (animationNode)
	{
		// Position
		GMVec3 currentPosition;
		if (!animationNode->positions.empty())
		{
			const auto& v = animationNode->positions;
			if (v.size() > 1)
			{
				GMfloat frameIdx = findIndex(animationTime, v);
				GMfloat nextFrameIdx = frameIdx;
				const auto& frame = v[frameIdx];
				const auto& nextFrame = v[nextFrameIdx];
				factor = (animationTime - frame.time) / (nextFrame.time - frame.time);
				currentPosition = Lerp(frame.value, nextFrame.value, factor);
			}
			else
			{
				currentPosition = v[0].value;
			}
		}

		// Rotation
		GMQuat currentRotation;
		if (!animationNode->rotations.empty())
		{
			const auto& v = animationNode->rotations;
			if (v.size() > 1)
			{
				GMfloat frameIdx = findIndex(animationTime, v);
				GMfloat nextFrameIdx = frameIdx + 1;
				const auto& frame = v[frameIdx];
				const auto& nextFrame = v[nextFrameIdx];
				factor = (animationTime - frame.time) / (nextFrame.time - frame.time);
				GMDuration diffTime = nextFrame.time - frame.time;
				currentRotation = Lerp(frame.value, nextFrame.value, factor);
			}
			else
			{
				currentRotation = v[0].value;
			}
		}

		// Scaling
		GMVec3 currentScaling;
		if (!animationNode->scalings.empty())
		{
			const auto& v = animationNode->scalings;
			if (v.size() > 1)
			{
				GMfloat frameIdx = findIndex(animationTime, v);
				GMfloat nextFrameIdx = frameIdx + 1;
				const auto& frame = v[frameIdx];
				const auto& nextFrame = v[nextFrameIdx];
				factor = (animationTime - frame.time) / (nextFrame.time - frame.time);
				GMDuration diffTime = nextFrame.time - frame.time;
				currentScaling = Lerp(frame.value, nextFrame.value, factor);
			}
			else
			{
				currentScaling = v[0].value;
			}
		}

		nodeTransformation = Scale(currentScaling) * QuatToMatrix(currentRotation) * Translate(currentPosition);
	}

	GMMat4 globalTransformation = nodeTransformation * parentTransformation;
	
	// 把变换结果保存
	auto& boneMapping = d->skeleton->getBones().getBoneNameIndexMap();
	auto& boneInfo = d->skeleton->getBones().getBones();
	if (boneMapping.find(nodeName) != boneMapping.end())
	{
		GMsize_t idx = boneMapping[nodeName];
		auto& bone = boneInfo[idx];
		bone.finalTransformation = bone.offsetMatrix * globalTransformation * d->globalInverseTransform;
	}

	for (auto& child : node->getChildren())
	{
		updateNode(animationTime, child, globalTransformation);
	}
}

const GMSkeletalAnimationNode* GMSkeletalAnimationEvaluator::findAnimationNode(const GMString& name)
{
	D(d);
	for (auto& node : d->animation->nodes)
	{
		if (node.name == name)
			return &node;
	}
	return nullptr;
}