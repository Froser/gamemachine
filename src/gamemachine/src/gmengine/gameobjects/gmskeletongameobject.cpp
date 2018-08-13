#include "stdafx.h"
#include "gmskeletongameobject.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"

void GMSkeletonGameObject::update(GMDuration dt)
{
	D(d);
	if (d->playing)
	{
		GMModels* models = getModels();
		if (!models)
			return;

		auto skeleton = models->getSkeleton();
		if (skeleton)
		{
			initAnimation();

			GMint frame0 = 0, frame1 = 0;
			GMfloat interpolate = 0;
			if (d->frame == AutoPlayFrame)
			{
				getAdjacentTwoFrames(dt, frame0, frame1, interpolate);
			}
			else
			{
				frame0 = d->frame;
				frame1 = d->frame + 1;
				interpolate = d->frameInterpolate;
			}
			skeleton->interpolateSkeletons(frame0, frame1, interpolate);

			const GMFrameSkeleton& frameSkeleton = skeleton->getAnimatedSkeleton();
			for (auto& mesh : models->getSkeleton()->getMeshes())
			{
				updateMesh(mesh, frameSkeleton);
			}

			if (d->drawBones)
				updateSkeleton();
		}
	}
}

void GMSkeletonGameObject::draw()
{
	D(d);
	if (d->drawSkin)
		GMGameObject::draw();
}

GMint GMSkeletonGameObject::getFramesCount()
{
	D(d);
	GMModels* models = getModels();
	if (!models)
		return 0;

	auto skeleton = models->getSkeleton();
	if (skeleton)
		return skeleton->getSkeletons().getNumFrames();

	return 0;
}

void GMSkeletonGameObject::createSkeletonBonesObject()
{
	D(d);
	GMModels* models = getModels();
	if (!models)
		return;

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

		GMMesh* mesh = new GMMesh(skeletonModel);
		initSkeletonBonesMesh(mesh);
		GM.createModelDataProxyAndTransfer(getContext(), skeletonModel);
		GM_ASSERT(getWorld());

		d->skeletonBonesObject.reset(new GMGameObject(GMAsset(GMAssetType::Model, skeletonModel)));
		d->skeletonBonesObject->setContext(getContext());
		getWorld()->addToRenderList(d->skeletonBonesObject.get());
	}
}

void GMSkeletonGameObject::setDrawBones(bool b)
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

void GMSkeletonGameObject::play()
{
	D(d);
	d->playing = true;
}

void GMSkeletonGameObject::pause()
{
	D(d);
	d->playing = false;
}

void GMSkeletonGameObject::reset(bool update)
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

void GMSkeletonGameObject::initAnimation()
{
	D(d);
	GMSkeleton* skeleton = getModels()->getSkeleton();
	GM_ASSERT(skeleton);
	if (d->frameDuration == 0)
	{
		d->frameDuration = 1.f / skeleton->getFrameRate();
	}

	if (d->animationDuration == 0)
	{
		// 计算播放全部动画需要的时间
		d->animationDuration = d->frameDuration * skeleton->getSkeletons().getNumFrames();
	}
}

void GMSkeletonGameObject::getAdjacentTwoFrames(GMDuration dt, REF GMint& frame0, REF GMint& frame1, REF GMfloat& interpolate)
{
	D(d);
	GMSkeleton* skeleton = getModels()->getSkeleton();
	GM_ASSERT(skeleton);
	if (skeleton->getSkeletons().getNumFrames() < 1)
		return;

	d->animationTime += dt;
	while (d->animationTime > d->animationDuration)
	{
		d->animationTime -= d->animationDuration;
	}

	while (d->animationTime < 0.0f)
	{
		d->animationTime += d->animationDuration;
	}

	// 找到当前所在的帧
	GMfloat frameNum = d->animationTime * static_cast<GMfloat>(skeleton->getFrameRate());
	frame0 = Floor(frameNum);
	frame1 = Ceil(frameNum);
	frame0 = frame0 % skeleton->getSkeletons().getNumFrames();
	frame1 = frame1 % skeleton->getSkeletons().getNumFrames();

	interpolate = Fmod(d->animationTime, d->frameDuration) / d->frameDuration;
}

void GMSkeletonGameObject::updateMesh(GMSkeletonMesh& mesh, const GMFrameSkeleton& frameSkeleton)
{
	static GMint numberOfProcessors = GM.getRunningStates().systemInfo.numberOfProcessors;

	GMModel* model = mesh.targetModel;
	GM_ASSERT(model);
	if (model->getUsageHint() == GMUsageHint::StaticDraw)
	{
		gm_error(gm_dbg_wrap("Cannot modify vertices because this is a static object."));
		return;
	}

	// 临时结构，用于缓存顶点、法线
	struct Vertex
	{
		GMVec3 position = Zero<GMVec3>();
		GMVec3 normal = Zero<GMVec3>();
		GMVec2 texcoord = Zero<GMVec2>();
	};

	auto modelDataProxy = model->getModelDataProxy();
	if (modelDataProxy)
	{
		modelDataProxy->beginUpdateBuffer(GMModelBufferType::VertexBuffer);
		GMVertex* modelVertices = static_cast<GMVertex*>(modelDataProxy->getBuffer());

		AlignedVector<Vertex> vertices;
		vertices.resize(mesh.vertices.size());
		GMAsync::blockedAsync(
			GMAsync::Async,
			numberOfProcessors,
			mesh.vertices.begin(),
			mesh.vertices.end(),
			[&vertices, &mesh, &frameSkeleton](auto begin, auto end) {
				GMint index = begin - mesh.vertices.begin();
				for (auto iter = begin; iter != end; ++iter)
				{
					auto& vert = *iter;
					Vertex vertex;
					GMVec3 pos = Zero<GMVec3>();
					// 每个顶点的坐标由结点的权重累计计算得到
					for (GMint i = 0; i < vert.weightCount; ++i)
					{
						const auto& weight = mesh.weights[vert.startWeight + i];
						const auto& joint = frameSkeleton.getJoints()[weight.jointIndex];
						GMVec3 rotationPos = weight.weightPosition * joint.getOrientation();
						vertex.position += (joint.getPosition() + rotationPos) * weight.weightBias;
					}
					vertex.texcoord = vert.texCoords;
					vertices[index++] = vertex;
				}
			}
		);

		GMAsync::blockedAsync(
			GMAsync::Async,
			numberOfProcessors,
			mesh.triangleIndices.begin(),
			mesh.triangleIndices.end(),
			[&vertices](auto begin, auto end) {
				for (auto iter = begin; iter != end; ++iter)
				{
					const auto& triIdx = *iter;
					GMVec3 v0 = vertices[triIdx[0]].position;
					GMVec3 v1 = vertices[triIdx[1]].position;
					GMVec3 v2 = vertices[triIdx[2]].position;
					GMVec3 normal = Cross(v2 - v0, v1 - v0);

					// 计算法线
					vertices[triIdx[0]].normal += normal;
					vertices[triIdx[1]].normal += normal;
					vertices[triIdx[2]].normal += normal;
				}
			}
		);

		GMAsync::blockedAsync(
			GMAsync::Async,
			numberOfProcessors,
			vertices.begin(),
			vertices.end(),
			[](auto begin, auto end) {
				for (auto iter = begin; iter != end; ++iter)
				{
					iter->normal = Normalize(iter->normal);
				}
			}
		);

		// 组装Vertex
		GMint vertexIndex = 0;
		for (const auto& triIdx : mesh.triangleIndices)
		{
			for (GMint i = 0; i < 3; ++i)
			{
				GMVertex& v = modelVertices[vertexIndex];
				const Vertex& vertexTemp = vertices[triIdx[i]];
				v.positions = { vertexTemp.position.getX(), vertexTemp.position.getY(), vertexTemp.position.getZ() };
				v.normals = { vertexTemp.normal.getX(), vertexTemp.normal.getY(), vertexTemp.normal.getZ() };
				v.texcoords = { vertexTemp.texcoord.getX(), vertexTemp.texcoord.getY() };
				++vertexIndex;
			}
		}

		modelDataProxy->endUpdateBuffer();
	}
}

void GMSkeletonGameObject::updateSkeleton()
{
	D(d);
	GMModels* models = getModels();
	if (!models)
		return;

	auto skeleton = models->getSkeleton();
	if (!skeleton)
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
	modelDataProxy->endUpdateBuffer();

	// 同步变换
	d->skeletonBonesObject->beginUpdateTransform();
	d->skeletonBonesObject->setTranslation(getTranslation());
	d->skeletonBonesObject->setRotation(getRotation());
	d->skeletonBonesObject->setScaling(getScaling());
	d->skeletonBonesObject->endUpdateTransform();
}

void GMSkeletonGameObject::initSkeletonBonesMesh(GMMesh* mesh)
{
	D(d);
	// 找到所有joint，连接成线
	GMModels* models = getModels();
	GM_ASSERT(models);
	if (!models)
		return;

	auto skeleton = models->getSkeleton();
	if (!skeleton)
		return;

	auto& joints = skeleton->getAnimatedSkeleton().getJoints();
	for (auto& joint : joints)
	{
		const auto& parentId = joint.getParentIndex();
		if (parentId == GMSkeletonJoint::RootIndex)
			continue;

		// 每个关节绑定2个顶点，绘制出一条直线
		mesh->vertex(GMVertex());
		mesh->vertex(GMVertex());
	}
}