#include "stdafx.h"
#include "gmskeletalgameobject.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"

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
		GMModels* models = getModels();
		if (!models)
			return;

		for (auto& model : models->getModels())
		{
			auto skeleton = model.getModel()->getSkeleton();
			if (skeleton)
			{
				auto animations = models->getAnimations();
				if (animations)
				{
					GMSkeletalAnimationEvaluator* evaluator = d->modelEvaluatorMap[model.getModel()];
					if (!evaluator)
						evaluator = d->modelEvaluatorMap[model.getModel()] = new GMSkeletalAnimationEvaluator(models->getRootNode(), skeleton);

					evaluator->setAnimation(animations->getAnimation(0)); //TODO
					evaluator->update(dt);
					updateModel(model.getModel(), evaluator);
				}
			}
		}
	}
}

void GMSkeletalGameObject::updateModel(GMModel* model, GMSkeletalAnimationEvaluator* evaluator)
{
	const auto& transforms = evaluator->getTransforms();

	// 遍历所有的顶点，找到其对应的骨骼，并乘以它的权重
	// TODO: 这一步可以在GPU运行，不过先在CPU运行
	if (model->getUsageHint() == GMUsageHint::StaticDraw)
	{
		gm_error(gm_dbg_wrap("Cannot modify vertices because this is a static object."));
		return;
	}

	// 临时结构，用于缓存顶点、法线
	struct Vertex
	{
		GMVec4 position = Zero<GMVec4>();
		GMVec3 normal = Zero<GMVec3>();
	};

	auto& vertices = model->getPackedVertices();

	auto modelDataProxy = model->getModelDataProxy();
	if (modelDataProxy)
	{
		auto& bones = model->getSkeleton()->getBones();
		auto& vertexData = bones.getVertexData();
		GM_ASSERT(vertexData.size() == vertices.size());
		const auto& transforms = evaluator->getTransforms();

		modelDataProxy->beginUpdateBuffer(GMModelBufferType::VertexBuffer);
		GMVertex* modelVertices = static_cast<GMVertex*>(modelDataProxy->getBuffer());

		// vertexData一一对应每一个modelVertex
		static GMMat4 zeroMat;
		GMFloat16 zeroF16 = { GMFloat4(0, 0, 0, 0), GMFloat4(0, 0, 0, 0), GMFloat4(0, 0, 0, 0), GMFloat4(0, 0, 0, 0) };
		zeroMat.setFloat16(zeroF16);

		for (GMsize_t i = 0; i < model->getPackedVertices().size(); ++i)
		{
			GMMat4 t = zeroMat;
			for (auto j = 0; j < GMSkeletalVertexBoneData::BonesPerVertex; ++j)
			{
				t += vertexData[i].getWeights()[j] * transforms[vertexData[i].getIds()[j]];
				//v.position = vertexData[i].getWeights()[j] * transforms[vertexData[i].getIds()[j]];
			}

			Vertex v;
			v.position = GMVec4(vertices[i].positions[0], vertices[i].positions[1], vertices[i].positions[2], 1) * t;
			v.normal = GMVec4(vertices[i].normals[0], vertices[i].normals[1], vertices[i].normals[2], 0) * t;

			modelVertices[i].positions = { v.position.getX(), v.position.getY(), v.position.getZ() };
			modelVertices[i].texcoords = vertices[i].texcoords;
			modelVertices[i].normals = { v.normal.getX(), v.normal.getY(), v.normal.getZ() };
		}

		modelDataProxy->endUpdateBuffer();
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
	GMModels* models = getModels();
	if (!models)
		return;
	/*
	auto skeleton = models->getSkeleton();
	if (skeleton && d->drawBones && !d->skeletonBonesObject)
	{
		// 创建连接骨骼的线条
		GMModel* skeletonModel = new GMModel();
		skeletonModel->getShader().setNoDepthTest(true);
		skeletonModel->getShader().setCull(GMS_Cull::None);
		skeletonModel->getShader().setVertexColorOp(GMS_VertexColorOp::Replace);
		skeletonModel->setPrimitiveTopologyMode(GMTopologyMode::Lines);
		skeletonModel->setUsageHint(GMUsageHint::DynamicDraw);

		GMMesh* part = new GMMesh(skeletonModel);
		initSkeletonBonesMesh(part);
		getContext()->getEngine()->createModelDataProxy(getContext(), skeletonModel);
		GM_ASSERT(getWorld());

		d->skeletonBonesObject.reset(new GMGameObject(GMAsset(GMAssetType::Model, skeletonModel)));
		d->skeletonBonesObject->setContext(getContext());
		getWorld()->addToRenderList(d->skeletonBonesObject.get());
	}
	*/
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
	d->animationTime = 0;

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

void GMSkeletalGameObject::updateSkeleton()
{
	D(d);
	GMModels* models = getModels();
	if (!models)
		return;

	/*
	auto skeleton = models->getSkeleton();
	if (!skeleton)
		return;
		*/

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

	/*
	auto& joints = skeleton->getAnimatedSkeleton().getJoints();
	for (auto& joint : joints)
	{
		const auto& parentId = joint.getParentIndex();
		if (parentId == GMSkeletonJoint::RootIndex)
			continue;

		GMVertex* vertex0 = verticesPtr++;
		GMVertex* vertex1 = verticesPtr++;
		const GMVec3& p0 = joint.getPosition();
		const GMVec3& p1 = joints[parentId].getPosition();
		vertex0->positions = { p0.getX(), p0.getY(), p0.getZ() };
		vertex0->color = color;
		vertex1->positions = { p1.getX(), p1.getY(), p1.getZ() };
		vertex1->color = color;
	}
	*/
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
	GMModels* models = getModels();
	GM_ASSERT(models);
	if (!models)
		return;

	for (auto& model : models->getModels())
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

	/*
	auto& joints = skeleton->getAnimatedSkeleton().getJoints();
	for (auto& joint : joints)
	{
		const auto& parentId = joint.getParentIndex();
		if (parentId == GMSkeletonJoint::RootIndex)
			continue;

		// 每个关节绑定2个顶点，绘制出一条直线
		part->vertex(GMVertex());
		part->vertex(GMVertex());
	}
	*/
}