#include "stdafx.h"
#include "gmanimationobjecthelper.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"

GMAnimationEvaluator::GMAnimationEvaluator(GMNode* root, GMSkeleton* skeleton)
{
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
	d->lastFrameIdx.clear();
}

void GMAnimationEvaluator::updateNode(GMDuration animationTime, GMNode* node, const GMMat4& parentTransformation)
{
	D(d);
	enum
	{
		Translation,
		Rotation,
		Scaling,
	};

	const GMString& nodeName = node->getName();
	const GMNodeAnimationNode* animationNode = findAnimationNode(nodeName);
	GMMat4 nodeTransformation = node->getTransformToParent();
	GMfloat factor = 0;

	GMfloat time = 0;
	if (d->animation->duration > 0)
		time = Fmod(animationTime, d->animation->duration);

	if (animationNode)
	{
		// Position
		GMVec3 currentPosition(0, 0, 0);
		if (!animationNode->positions.empty())
		{
			const auto& v = animationNode->positions;
			if (v.size() > 1)
			{
				GMuint32 frameIdx = (time >= d->lastTime) ? d->lastFrameIdx[animationNode][Translation] : 0;
				while (frameIdx < animationNode->positions.size() - 1)
				{
					if (time < animationNode->positions[frameIdx + 1].time)
						break;
					frameIdx++;
				}
				
				d->lastFrameIdx[animationNode][Translation] = frameIdx;
				GMfloat nextFrameIdx = (frameIdx + 1) % animationNode->positions.size();

				const auto& frame = v[frameIdx];
				const auto& nextFrame = v[nextFrameIdx];
				GMfloat diffTime = nextFrame.time - frame.time;
				if (diffTime > 0)
				{
					factor = (animationTime - frame.time) / diffTime;
					currentPosition = Lerp(frame.value, nextFrame.value, factor);
				}
				else
				{
					currentPosition = frame.value;
				}
			}
			else
			{
				currentPosition = v[0].value;
			}
		}

		// Rotation
		GMQuat currentRotation = Identity<GMQuat>();
		if (!animationNode->rotations.empty())
		{
			const auto& v = animationNode->rotations;
			if (v.size() > 1)
			{
				GMuint32 frameIdx = (time >= d->lastTime) ? d->lastFrameIdx[animationNode][Rotation] : 0;
				while (frameIdx < animationNode->rotations.size() - 1)
				{
					if (time < animationNode->rotations[frameIdx + 1].time)
						break;
					frameIdx++;
				}

				d->lastFrameIdx[animationNode][Rotation] = frameIdx;
				GMfloat nextFrameIdx = (frameIdx + 1) % animationNode->rotations.size();

				const auto& frame = v[frameIdx];
				const auto& nextFrame = v[nextFrameIdx];
				GMfloat diffTime = nextFrame.time - frame.time;
				if (diffTime > 0)
				{
					factor = (animationTime - frame.time) / diffTime;
					currentRotation = Lerp(frame.value, nextFrame.value, factor);
				}
				else
				{
					currentRotation = frame.value;
				}
			}
			else
			{
				currentRotation = v[0].value;
			}
		}

		// Scaling
		GMVec3 currentScaling(1, 1, 1);
		if (!animationNode->scalings.empty())
		{
			const auto& v = animationNode->scalings;
			if (v.size() > 1)
			{
				GMuint32 frameIdx = (time >= d->lastTime) ? d->lastFrameIdx[animationNode][Scaling] : 0;
				while (frameIdx < animationNode->scalings.size() - 1)
				{
					if (time < animationNode->scalings[frameIdx + 1].time)
						break;
					frameIdx++;
				}

				d->lastFrameIdx[animationNode][Scaling] = frameIdx;
				GMfloat nextFrameIdx = (frameIdx + 1) % animationNode->scalings.size();

				const auto& frame = v[frameIdx];
				const auto& nextFrame = v[nextFrameIdx];
				GMfloat diffTime = nextFrame.time - frame.time;
				if (diffTime > 0)
				{
					factor = (animationTime - frame.time) / diffTime;
					currentScaling = Lerp(frame.value, nextFrame.value, factor);
				}
				else
				{
					currentScaling = frame.value;
				}
			}
			else
			{
				currentScaling = v[0].value;
			}
		}
		nodeTransformation = Scale(currentScaling) * QuatToMatrix(currentRotation) * Translate(currentPosition);
	}

	d->lastTime = time;

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

GMAnimationGameObjectHelper::GMAnimationGameObjectHelper(GMGameObject* host)
{
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
