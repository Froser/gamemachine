#include "stdafx.h"
#include "gmanimationobjecthelper.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"

BEGIN_NS

namespace
{
	template <typename T>
	GMsize_t findIndex(GMDuration animationTime, const AlignedVector<GMNodeAnimationKeyframe<T>>& frames)
	{
		GM_ASSERT(frames.size() > 0);
		for (GMsize_t i = 0; i < frames.size() - 1; ++i)
		{
			const GMNodeAnimationKeyframe<T>& frame = frames[i + 1];
			if (animationTime < frame.time)
				return i;
		}

		GM_ASSERT(false);
		return 0;
	}
}

GM_PRIVATE_OBJECT_ALIGNED(GMAnimationEvaluator)
{
	const GMNodeAnimation* animation = nullptr;
	GMDuration duration = 0;
	AlignedVector<GMMat4> transforms;
	GMSkeletonPtr skeleton = nullptr;
	GMNodePtr rootNode = nullptr;
	GMMat4 globalInverseTransform;
};

GM_DEFINE_PROPERTY(GMAnimationEvaluator, GMSkeletonPtr, Skeleton, skeleton)
GM_DEFINE_PROPERTY(GMAnimationEvaluator, GMNodePtr, RootNode, rootNode)
GM_DEFINE_PROPERTY(GMAnimationEvaluator, GMNodeAnimationPtr, Animation, animation)
GM_DEFINE_GETTER(GMAnimationEvaluator, AlignedVector<GMMat4>, Transforms, transforms)
GMAnimationEvaluator::GMAnimationEvaluator(GMNode* root, GMSkeleton* skeleton)
{
	GM_CREATE_DATA();

	D(d);
	setRootNode(root);
	setSkeleton(skeleton);
	d->globalInverseTransform = Inverse(root->getTransformToParent());
}

void GMAnimationEvaluator::update(GMDuration dt)
{
	D(d);
	d->duration += dt;

	GMfloat ticks = d->duration * d->animation->frameRate;
	GMDuration animationTime = Fmod(ticks, d->animation->duration);

	updateNode(animationTime, d->rootNode, Identity<GMMat4>());
	AlignedVector<GMMat4>& transforms = d->transforms;

	if (d->skeleton)
	{
		// 在骨骼动画中，transforms表示每个骨骼的变换
		GMsize_t boneCount = d->skeleton->getBones().getBones().size();
		transforms.resize(boneCount);

		for (GMsize_t i = 0; i < boneCount; ++i)
		{
			transforms[i] = d->skeleton->getBones().getBones()[i].finalTransformation;
		}
	}
}

void GMAnimationEvaluator::reset()
{
	D(d);
	d->duration = 0;
}

void GMAnimationEvaluator::updateNode(GMDuration animationTime, GMNode* node, const GMMat4& parentTransformation)
{
	D(d);
	const GMString& nodeName = node->getName();
	const GMNodeAnimationNode* animationNode = findAnimationNode(nodeName);
	GMMat4 nodeTransformation = node->getTransformToParent();
	GMfloat factor = 0;

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
				GMfloat nextFrameIdx = frameIdx + 1;
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
	node->setGlobalTransform(globalTransformation);

	// 把变换结果保存
	if (d->skeleton)
	{
		// 如果是骨骼动画，结果保存到骨骼节点
		auto& boneMapping = d->skeleton->getBones().getBoneNameIndexMap();
		auto& boneInfo = d->skeleton->getBones().getBones();
		if (boneMapping.find(nodeName) != boneMapping.end())
		{
			GMsize_t idx = boneMapping[nodeName];
			auto& bone = boneInfo[idx];
			bone.finalTransformation = bone.offsetMatrix * globalTransformation * d->globalInverseTransform;
		}
	}

	for (auto& child : node->getChildren())
	{
		updateNode(animationTime, child, globalTransformation);
	}
}

const GMNodeAnimationNode* GMAnimationEvaluator::findAnimationNode(const GMString& name)
{
	D(d);
	for (auto& node : d->animation->nodes)
	{
		if (node.name == name)
			return &node;
	}
	return nullptr;
}

GM_PRIVATE_OBJECT_ALIGNED(GMAnimationGameObjectHelper)
{
	enum { AutoPlayFrame = -1 };

	bool playing = true;
	GMGameObject* host = nullptr;
	GMVec4 skeletonColor = GMVec4(0, 1, 0, 1);
	Map<GMModel*, GMAnimationEvaluator*> modelEvaluatorMap;
	Vector<GMString> nameList;
	GMsize_t animationIndex = 0;
};

GMAnimationGameObjectHelper::GMAnimationGameObjectHelper(GMGameObject* host)
{
	GM_CREATE_DATA();

	D(d);
	d->host = host;
}

GMAnimationGameObjectHelper::~GMAnimationGameObjectHelper()
{
	D(d);
	for (auto& kv : d->modelEvaluatorMap)
	{
		GM_delete(kv.second);
	}
}

void GMAnimationGameObjectHelper::update(GMDuration dt)
{
	D(d);
	if (d->playing)
	{
		GMScene* scene = d->host->getScene();
		if (!scene)
			return;

		for (auto& model : scene->getModels())
		{
			auto animations = scene->getAnimations();
			if (animations)
			{
				GMSkeleton* skeleton = model.getModel()->getSkeleton();
				GMAnimationEvaluator* evaluator = d->modelEvaluatorMap[model.getModel()];
				if (!evaluator)
					evaluator = d->modelEvaluatorMap[model.getModel()] = new GMAnimationEvaluator(scene->getRootNode(), skeleton);

				evaluator->setAnimation(animations->getAnimation(d->animationIndex));
				evaluator->update(dt);

				if (skeleton)
				{
					auto& transforms = evaluator->getTransforms();
					model.getModel()->getBoneTransformations().swap(transforms);
				}
				else
				{
					// global transform 都记录在了对应的node上，找到对应的node，赋值给boneTransformations[0]
					for (auto node : model.getModel()->getNodes())
					{
						GM_ASSERT(node);
						auto& modelTransforms = model.getModel()->getBoneTransformations();
						modelTransforms.clear();
						modelTransforms.push_back(node->getGlobalTransform());
					}
				}
			}
		}
	}
}

void GMAnimationGameObjectHelper::play()
{
	D(d);
	d->playing = true;
}

void GMAnimationGameObjectHelper::pause()
{
	D(d);
	d->playing = false;
}

void GMAnimationGameObjectHelper::reset(bool update)
{
	D(d);
	for (auto& evaluator : d->modelEvaluatorMap)
	{
		if (evaluator.second)
			evaluator.second->reset();
	}

	if (update)
	{
		// 更新一帧
		bool t = isPlaying();
		play();
		this->update(0);
		if (t)
			play();
		else
			pause();
	}
}

GMsize_t GMAnimationGameObjectHelper::getAnimationCount()
{
	D(d);
	GMScene* scene = d->host->getScene();
	if (!scene)
		return 0;

	if (!scene->hasAnimation())
		return 0;

	return scene->getAnimations()->getAnimationCount();
}

void GMAnimationGameObjectHelper::setAnimation(GMsize_t index)
{
	D(d);
	if (index >= getAnimationCount())
	{
		gm_error(gm_dbg_wrap("Animation index overflow. Max count is {0}"), GMString(gm_sizet_to_int(getAnimationCount())));
		index = 0;
	}
	d->animationIndex = index;
}

Vector<GMString> GMAnimationGameObjectHelper::getAnimationNames()
{
	D(d);
	GMScene* scene = d->host->getScene();
	if (!scene)
		return Vector<GMString>();

	if (!scene->hasAnimation())
		return Vector<GMString>();

	if (d->nameList.empty())
	{
		for (GMsize_t i = 0u; i < scene->getAnimations()->getAnimationCount(); ++i)
		{
			auto animation = scene->getAnimations()->getAnimation(i);
			d->nameList.push_back(animation->name);
		}
	}
	return d->nameList;
}

GMsize_t GMAnimationGameObjectHelper::getAnimationIndexByName(const GMString& name)
{
	D(d);
	GMScene* scene = d->host->getScene();
	if (!scene)
		return GMGameObject::InvalidIndex;

	if (!scene->hasAnimation())
		return GMGameObject::InvalidIndex;

	Vector<GMString> animationNames = getAnimationNames();
	auto findResult = std::find(animationNames.begin(), animationNames.end(), name);
	if (findResult == animationNames.end())
		return GMGameObject::InvalidIndex;

	return findResult - animationNames.begin();
}

bool GMAnimationGameObjectHelper::isPlaying() GM_NOEXCEPT
{
	D(d);
	return d->playing;
}

END_NS