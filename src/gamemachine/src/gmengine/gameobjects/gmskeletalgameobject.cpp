#include "stdafx.h"
#include "gmskeletalgameobject.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"

namespace
{
	template <typename T>
	GMsize_t findIndex(GMDuration animationTime, const AlignedVector<GMSkeletonAnimationKeyframe<T>>& frames)
	{
		GM_ASSERT(frames.size() > 0);
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
	GMDuration animationTime = Fmod(ticks, d->animation->duration);

	updateNode(animationTime, d->rootNode, Identity<GMMat4>());
	AlignedVector<GMMat4>& transforms = d->transforms;

	GMsize_t boneCount = d->skeleton->getBones().getBones().size();
	transforms.resize(boneCount);

	for (GMsize_t i = 0; i < boneCount; ++i)
	{
		transforms[i] = d->skeleton->getBones().getBones()[i].finalTransformation;
	}
}

void GMSkeletalAnimationEvaluator::reset()
{
	D(d);
	d->duration = 0;
}

void GMSkeletalAnimationEvaluator::updateNode(GMDuration animationTime, GMSkeletalNode* node, const GMMat4& parentTransformation)
{
	D(d);
	const GMString& nodeName = node->getName();
	const GMMat4& transformation = node->getTransformToParent();
	const GMSkeletalAnimationNode* animationNode = findAnimationNode(nodeName);
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
GMSkeletalGameObject::~GMSkeletalGameObject()
{
	D(d);
	for (auto& kv : d->modelEvaluatorMap)
	{
		GM_delete(kv.second);
	}
}

void GMSkeletalGameObject::update(GMDuration dt)
{
	D(d);
	if (d->playing)
	{
		GMScene* scene = getScene();
		if (!scene)
			return;

		for (auto& model : scene->getModels())
		{
			auto skeleton = model.getModel()->getSkeleton();
			if (skeleton)
			{
				auto animations = scene->getAnimations();
				if (animations)
				{
					GMSkeletalAnimationEvaluator* evaluator = d->modelEvaluatorMap[model.getModel()];
					if (!evaluator)
						evaluator = d->modelEvaluatorMap[model.getModel()] = new GMSkeletalAnimationEvaluator(scene->getRootNode(), skeleton);

					evaluator->setAnimation(animations->getAnimation(d->animationIndex));
					evaluator->update(dt);

					auto& transforms = evaluator->getTransforms();
					model.getModel()->getBoneTransformations().swap(transforms);
				}
			}
		}
	}
}

void GMSkeletalGameObject::draw()
{
	D(d);
	if (d->drawSkin)
		GMGameObject::draw();
}

void GMSkeletalGameObject::createSkeletonBonesObject()
{
	D(d);
	GMScene* scene = getScene();
	if (!scene)
		return;
}

void GMSkeletalGameObject::setDrawBones(bool b)
{
	D(d);
	d->drawBones = b;

	if (d->skeletonBonesObject)
	{
		GMModel* model = d->skeletonBonesObject->getModel();
		GM_ASSERT(model);
		if (!model)
			return;

		model->getShader().setDiscard(!b);
	}
}

void GMSkeletalGameObject::play()
{
	D(d);
	d->playing = true;
}

void GMSkeletalGameObject::pause()
{
	D(d);
	d->playing = false;
}

void GMSkeletalGameObject::reset(bool update)
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

GMsize_t GMSkeletalGameObject::getAnimationCount()
{
	D(d);
	GMScene* scene = getScene();
	if (!scene)
		return 0;

	if (!scene->hasAnimation())
		return 0;

	return scene->getAnimations()->getAnimationCount();
}

void GMSkeletalGameObject::updateSkeleton()
{
	D(d);
	GMScene* scene = getScene();
	if (!scene)
		return;

	if (!d->drawBones)
		return;

	if (!d->skeletonBonesObject)
		createSkeletonBonesObject();

	if (!d->skeletonBonesObject)
		return;

	GMModel* bonesModel = d->skeletonBonesObject->getModel();
	if (!bonesModel)
		return;

	const GMVec4& sc = getSkeletonColor();
	Array<GMfloat, 4> color;
	CopyToArray(sc, &color[0]);

	GMModelDataProxy* modelDataProxy = bonesModel->getModelDataProxy();
	modelDataProxy->beginUpdateBuffer(GMModelBufferType::VertexBuffer);
	GMVertex* const vertices = static_cast<GMVertex*>(modelDataProxy->getBuffer());
	GMVertex* verticesPtr = vertices;

	modelDataProxy->endUpdateBuffer();

	// 同步变换
	d->skeletonBonesObject->beginUpdateTransform();
	d->skeletonBonesObject->setTranslation(getTranslation());
	d->skeletonBonesObject->setRotation(getRotation());
	d->skeletonBonesObject->setScaling(getScaling());
	d->skeletonBonesObject->endUpdateTransform();
}

void GMSkeletalGameObject::initSkeletonBonesMesh(GMPart* part)
{
	D(d);
	// 找到所有joint，连接成线
	GMScene* scene = getScene();
	GM_ASSERT(scene);
	if (!scene)
		return;

	for (auto& model : scene->getModels())
	{
		auto skeleton = model.getModel()->getSkeleton();
		if (!skeleton)
			return;
		const auto& bones = skeleton->getBones();
		for (const auto& bone : bones.getBones())
		{
			// 每个关节绑定2个顶点，绘制出一条直线
			part->vertex(GMVertex());
			part->vertex(GMVertex());
		}
	}
}