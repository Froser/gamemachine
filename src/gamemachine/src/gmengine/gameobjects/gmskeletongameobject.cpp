#include "stdafx.h"
#include "gmskeletongameobject.h"

void GMSkeletonGameObject::update(GMDuration dt)
{
	D(d);
	GMModels& models = getModels();
	auto skeleton = models.getSkeleton();
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
		for (auto& mesh : models.getSkeleton()->getMeshes())
		{
			updateMesh(mesh, frameSkeleton);
		}
	}
}

GMint GMSkeletonGameObject::getFramesCount()
{
	D(d);
	GMModels& models = getModels();
	auto skeleton = models.getSkeleton();
	if (skeleton)
		return skeleton->getSkeletons().getNumFrames();

	return 0;
}

void GMSkeletonGameObject::initAnimation()
{
	D(d);
	GMSkeleton* skeleton = getModels().getSkeleton();
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
	GMSkeleton* skeleton = getModels().getSkeleton();
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

		Vector<Vertex> vertices;
		vertices.reserve(mesh.vertices.size());
		for (const auto& vert : mesh.vertices)
		{
			Vertex vertex;
			// 每个顶点的坐标由结点的权重累计计算得到
			for (GMint i = 0; i < vert.weightCount; ++i)
			{
				const auto& weight = mesh.weights[vert.startWeight + i];
				const auto& joint = frameSkeleton.getJoints()[weight.jointIndex];
				GMVec3 rotationPos = weight.weightPosition * joint.getOrientation();
				vertex.position += (joint.getPosition() + rotationPos) * weight.weightBias;
				vertex.normal += vert.normal * joint.getOrientation() * weight.weightBias;
			}
			vertex.normal = Normalize(vertex.normal);
			vertex.texcoord = vert.texCoords;
			
			vertices.push_back(vertex);
		}

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