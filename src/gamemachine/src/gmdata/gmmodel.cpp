#include "stdafx.h"
#include "gmmodel.h"
#include <linearmath.h>
#include <algorithm>
#include <iterator>
#include "foundation/gamemachine.h"

#define TO_VEC3(i) GMVec3((i)[0], (i)[1], (i)[2])
#define TO_VEC2(i) GMVec2((i)[0], (i)[1])

BEGIN_NS

GMSceneAsset GMScene::createSceneFromSingleModel(GMModelAsset modelAsset)
{
	GMScene* scene = new GMScene();
	scene->addModelAsset(modelAsset);
	return GMAsset(GMAssetType::Scene, scene);
}

GM_PRIVATE_OBJECT_UNALIGNED(GMScene)
{
	Vector<GMAsset> models;
	GMOwnedPtr<GMSkeletalAnimations> animations;
	GMOwnedPtr<GMNode> root;
	GMAnimationType animationType = GMAnimationType::NoAnimation;
};

GM_DEFINE_PROPERTY(GMScene, Vector<GMAsset>, Models, models)
GM_DEFINE_PROPERTY(GMScene, GMAnimationType, AnimationType, animationType)
GMScene::GMScene()
{
	GM_CREATE_DATA(GMScene);
}

void GMScene::addModelAsset(GMModelAsset model)
{
	D(d);
	GM_ASSERT(model.getModel());
	d->models.push_back(model);
}

void GMScene::swap(GMScene* scene)
{
	D(d);
	D_OF(d_rhs, scene);
	using std::swap;
	d->models.swap(d_rhs->models);
	d->root.swap(d_rhs->root);
}

bool GMScene::hasAnimation() GM_NOEXCEPT
{
	D(d);
	return !!d->animations;
}

GMNode* GMScene::getRootNode() GM_NOEXCEPT
{
	D(d);
	return d->root.get();
}

GMModel* GMScene::operator[](GMsize_t i)
{
	D(d);
	return d->models[i].getModel();
}

void GMScene::setRootNode(GMNode* root)
{
	D(d);
	d->root.reset(root);
}

void GMScene::setAnimations(AUTORELEASE GMSkeletalAnimations* animations)
{
	D(d);
	GM_ASSERT(d->animationType != GMAnimationType::NoAnimation);
	d->animations.reset(animations);
}

GMSkeletalAnimations* GMScene::getAnimations() GM_NOEXCEPT
{
	D(d);
	return d->animations.get();
}

bool GMScene::isEmpty() GM_NOEXCEPT
{
	D(d);
	return d->models.empty();
}

GM_PRIVATE_OBJECT_UNALIGNED(GMModelDataProxy)
{
	const IRenderContext* context = nullptr;
	GMModel* model = nullptr;
};

GMModelDataProxy::GMModelDataProxy(const IRenderContext* context, GMModel* obj)
{
	GM_CREATE_DATA(GMModelDataProxy);
	D(d);
	d->context = context;
	d->model = obj;
}

const IRenderContext* GMModelDataProxy::getContext()
{
	D(d);
	return d->context;
}

GMModel* GMModelDataProxy::getModel()
{
	D(d);
	return d->model;
}

void GMModelDataProxy::prepareTangentSpace()
{
	GMModel* model = getModel();
	if (model->getDrawMode() == GMModelDrawMode::Vertex)
	{
		for (auto& part : model->getParts())
		{
			if (model->getShader().getTextureList().getTextureSampler(GMTextureType::NormalMap).getFrameCount() > 0)
				part->calculateTangentSpace(model->getPrimitiveTopologyMode());
		}
	}
	else
	{
		for (auto& part : model->getParts())
		{
			part->invalidateTangentSpace();
		}
	}
}

void GMModelDataProxy::packVertices(Vector<GMVertex>& vertices)
{
	GMModel* model = getModel();
	GMParts& parts = model->getParts();
	GMuint32 offset = 0;
	for (auto& part : parts)
	{
		for (auto& vertex : part->vertices())
		{
			vertices.push_back(vertex);
		}
	}
}

void GMModelDataProxy::packIndices(Vector<GMuint32>& indices)
{
	GMModel* model = getModel();
	GMParts& parts = model->getParts();
	GMuint32 offset = 0;
	for (auto& part : parts)
	{
		for (GMuint32 index : part->indices())
		{
			indices.push_back(index + offset);
		}

		// 每个part按照自己的坐标排序，因此每个part都应该在总缓存里面加上偏移
		offset += gm_sizet_to_uint(part->vertices().size());
	}
}

void GMModelDataProxy::prepareParentModel()
{
	D(d);
	// 如果有父model，表示共用同一个顶点数据
	GMModel* parent = d->model->getParentModel();
	if (parent)
	{
		if (!parent->getModelDataProxy())
			d->context->getEngine()->createModelDataProxy(d->context, parent, true);
		parent->getModelDataProxy()->prepareParentModel();

		// 参考 GMModel::GMModel(GMModelAsset parentAsset)
		d->model->setModelBuffer(parent->getModelBuffer());
		d->model->setPrimitiveTopologyMode(parent->getPrimitiveTopologyMode());
		d->model->setVerticesCount(parent->getVerticesCount());
	}
}

GM_PRIVATE_OBJECT_UNALIGNED(GMModel)
{
	GMUsageHint hint = GMUsageHint::StaticDraw;
	GMParts parts;
	GMOwnedPtr<GMModelDataProxy> modelDataProxy;
	GMShader shader;
	GMModelBuffer* modelBuffer = nullptr;
	GMModelDrawMode drawMode = GMModelDrawMode::Vertex;
	GMModelType type = GMModelType::Model3D;
	GMTopologyMode mode = GMTopologyMode::Triangles;
	GMsize_t verticesCount = 0;
	bool needTransfer = true;
	GMRenderTechniqueID techniqueId = 0;
	GMModelAsset parentAsset;
	GMOwnedPtr<GMSkeleton> skeleton;
	Vector<GMNode*> nodes;
	// 骨骼变换矩阵，对于无骨骼的model，使用首个元素表示变换。
	AlignedVector<GMMat4> boneTransformations;
};

GM_DEFINE_PROPERTY(GMModel, GMTopologyMode, PrimitiveTopologyMode, mode);
GM_DEFINE_PROPERTY(GMModel, GMModelType, Type, type);
GM_DEFINE_PROPERTY(GMModel, GMShader, Shader, shader);
GM_DEFINE_PROPERTY(GMModel, GMsize_t, VerticesCount, verticesCount);
GM_DEFINE_PROPERTY(GMModel, GMModelDrawMode, DrawMode, drawMode);
GM_DEFINE_PROPERTY(GMModel, GMRenderTechniqueID, TechniqueId, techniqueId);
GM_DEFINE_PROPERTY(GMModel, Vector<GMNode*>, Nodes, nodes)
GM_DEFINE_PROPERTY(GMModel, AlignedVector<GMMat4>, BoneTransformations, boneTransformations)

GMModel::GMModel()
{
	GM_CREATE_DATA(GMModel);
	D(d);
	d->modelBuffer = new GMModelBuffer();
}

GMModel::GMModel(GMModelAsset parentAsset)
{
	GM_CREATE_DATA(GMModel);
	D(d);
	GM_ASSERT(parentAsset.getModel());
	GMModel* parentModel = parentAsset.getModel();
	while (parentModel->getParentModel())
	{
		parentModel = getParentModel(); //拿到最根部的Model
	}
	d->parentAsset = parentAsset;
	
	parentModel = getParentModel();
	setShader(parentModel->getShader());

	setModelBuffer(parentModel->getModelBuffer());
	setDrawMode(parentModel->getDrawMode());
	setPrimitiveTopologyMode(parentModel->getPrimitiveTopologyMode());
	setVerticesCount(parentModel->getVerticesCount());

	doNotTransferAnymore();
}

GMModel::~GMModel()
{
	D(d);
	releaseModelBuffer();
	GM_delete(d->parts);
}

GMModelDataProxy* GMModel::getModelDataProxy() GM_NOEXCEPT
{
	D(d);
	return d->modelDataProxy.get();
}

void GMModel::setModelDataProxy(AUTORELEASE GMModelDataProxy* modelDataProxy)
{
	D(d);
	d->modelDataProxy.reset(modelDataProxy);
}

GMParts& GMModel::getParts() GM_NOEXCEPT
{
	D(d);
	return d->parts;
}

bool GMModel::isNeedTransfer() GM_NOEXCEPT
{
	D(d);
	return d->needTransfer;
}

void GMModel::doNotTransferAnymore() GM_NOEXCEPT
{
	D(d);
	d->needTransfer = false;
}

void GMModel::setUsageHint(GMUsageHint hint) GM_NOEXCEPT
{
	D(d);
	d->hint = hint;
}

GMUsageHint GMModel::getUsageHint() GM_NOEXCEPT
{
	D(d);
	return d->hint;
}

GMModel* GMModel::getParentModel() GM_NOEXCEPT
{
	D(d);
	return d->parentAsset.getModel();
}

GMSkeleton* GMModel::getSkeleton() GM_NOEXCEPT
{
	D(d);
	return d->skeleton.get();
}

void GMModel::setSkeleton(AUTORELEASE GMSkeleton* skeleton)
{
	D(d);
	d->skeleton.reset(skeleton);
}

void GMModel::setModelBuffer(AUTORELEASE GMModelBuffer* mb)
{
	D(d);
	if (d->modelBuffer != mb)
	{
		releaseModelBuffer();
		d->modelBuffer = mb;
		d->modelBuffer->addRef();
	}
}

GMModelBuffer* GMModel::getModelBuffer()
{
	D(d);
	return d->modelBuffer;
}

void GMModel::releaseModelBuffer()
{
	D(d);
	if (d->modelBuffer)
	{
		d->modelBuffer->releaseRef();
	}
}

void GMModel::addPart(GMPart* part)
{
	D(d);
	d->parts.push_back(part);
}

GM_PRIVATE_OBJECT_UNALIGNED(GMModelBuffer)
{
	GMModelBufferData buffer = { 0 };
	GMAtomic<GMint32> ref;
	GMModelDataProxy* modelDataProxy = nullptr;
};

GMModelBuffer::GMModelBuffer()
{
	GM_CREATE_DATA(GMModelBuffer);
	D(d);
	d->ref = 1;
	// 创建一个空的proxy，用于dispose
	// 它本身不会包含任何数据
	GM.getFactory()->createModelDataProxy(nullptr, nullptr, &d->modelDataProxy);
}

GMModelBuffer::~GMModelBuffer()
{
	D(d);
	GM_delete(d->modelDataProxy);
}

void GMModelBuffer::dispose()
{
	D(d);
	d->modelDataProxy->dispose(this);
}

void GMModelBuffer::addRef()
{
	D(d);
	++d->ref;
}

void GMModelBuffer::releaseRef()
{
	D(d);
	--d->ref;
	if (d->ref <= 0)
	{
		dispose();
		delete this;
	}
}

const GMModelBufferData& GMModelBuffer::getMeshBuffer()
{
	D(d);
	return d->buffer;
}

void GMModelBuffer::setData(const GMModelBufferData& bufferData)
{
	D(d);
	d->buffer = bufferData;
}

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT_UNALIGNED(GMPart)
{
	GMVertices vertices;
	GMIndices indices;
};

GMPart::GMPart(GMModel* parent)
{
	GM_CREATE_DATA(GMPart);
	parent->addPart(this);
}

void GMPart::clear()
{
	D(d);
	GMClearSTLContainer(d->vertices);
	GMClearSTLContainer(d->indices);
}

void GMPart::vertex(const GMVertex& v)
{
	D(d);
	d->vertices.push_back(v);
}

void GMPart::index(GMuint32 index)
{
	D(d);
	d->indices.push_back(index);
}

void GMPart::invalidateTangentSpace()
{
	D(d);
	for (auto& vertex : d->vertices)
	{
		vertex.tangents[0] = vertex.bitangents[0] = 
		vertex.tangents[1] = vertex.bitangents[1] = 
		vertex.tangents[2] = vertex.bitangents[2] = InvalidTangentSpace;
	}
}

void GMPart::swap(GMVertices& vertex)
{
	D(d);
	d->vertices.swap(vertex);
}

void GMPart::swap(GMIndices& indices)
{
	D(d);
	d->indices.swap(indices);
}

const GMIndices& GMPart::indices()
{
	D(d);
	return d->indices;
}

const GMVertices& GMPart::vertices()
{
	D(d);
	return d->vertices;
}

void GMPart::calculateTangentSpace(GMTopologyMode topologyMode)
{
	D(d);
	if (topologyMode == GMTopologyMode::Lines)
		return;

	for (GMuint32 i = 0; i < d->vertices.size(); i++)
	{
		GMVec3 e0, e1, e2;
		GMVec2 uv0, uv1, uv2;
		GMVertex& currentVertex = d->vertices[i];
		if (topologyMode == GMTopologyMode::Triangles)
		{
			GMuint32 startIndex = i / 3 * 3;
			GMuint32 vertexIndices[3] = { startIndex, startIndex + 1, startIndex + 2 };
			GMuint32 indexInTriangle = i % 3;
			e0 = TO_VEC3(d->vertices[vertexIndices[indexInTriangle % 3]].positions);
			e1 = TO_VEC3(d->vertices[vertexIndices[(indexInTriangle + 1) % 3]].positions);
			e2 = TO_VEC3(d->vertices[vertexIndices[(indexInTriangle + 2) % 3]].positions);
			uv0 = TO_VEC2(d->vertices[vertexIndices[indexInTriangle % 3]].texcoords);
			uv1 = TO_VEC2(d->vertices[vertexIndices[(indexInTriangle + 1) % 3]].texcoords);
			uv2 = TO_VEC2(d->vertices[vertexIndices[(indexInTriangle + 2) % 3]].texcoords);
		}
		else
		{
			GM_ASSERT(topologyMode == GMTopologyMode::TriangleStrip);
			if (i < 3)
			{
				GMuint32 vertexIndices[3] = { 0, 1, 2 };
				e0 = TO_VEC3(d->vertices[vertexIndices[i % 3]].positions);
				e1 = TO_VEC3(d->vertices[vertexIndices[(i + 1) % 3]].positions);
				e2 = TO_VEC3(d->vertices[vertexIndices[(i + 2) % 3]].positions);
				uv0 = TO_VEC2(d->vertices[vertexIndices[i % 3]].texcoords);
				uv1 = TO_VEC2(d->vertices[vertexIndices[(i + 1) % 3]].texcoords);
				uv2 = TO_VEC2(d->vertices[vertexIndices[(i + 2) % 3]].texcoords);
			}
			else
			{
				// 使用前2个顶点
				e0 = TO_VEC3(d->vertices[i].positions);
				e1 = TO_VEC3(d->vertices[i - 1].positions);
				e2 = TO_VEC3(d->vertices[i - 2].positions);
				uv0 = TO_VEC2(d->vertices[i].texcoords);
				uv1 = TO_VEC2(d->vertices[i - 1].texcoords);
				uv2 = TO_VEC2(d->vertices[i - 2].texcoords);
			}
		}

		GMVec3 E1 = e1 - e0;
		GMVec3 E2 = e2 - e0;
		GMVec2 deltaUV1 = uv1 - uv0;
		GMVec2 deltaUV2 = uv2 - uv0;
		const GMfloat& t1 = deltaUV1.getX();
		const GMfloat& b1 = deltaUV1.getY();
		const GMfloat& t2 = deltaUV2.getX();
		const GMfloat& b2 = deltaUV2.getY();

		GMFloat4 f4_E1, f4_E2;
		E1.loadFloat4(f4_E1);
		E2.loadFloat4(f4_E2);

		GMfloat s = 1.0f / (t1*b2 - b1*t2);

		GMfloat tangents[3] = {
			s * (b2 * f4_E1[0] - b1 * f4_E2[0]),
			s * (b2 * f4_E1[1] - b1 * f4_E2[1]),
			s * (b2 * f4_E1[2] - b1 * f4_E2[2])
		};
		GMfloat bitangents[3] = {
			s * (-t2 * f4_E1[0] + t1 * f4_E2[0]),
			s * (-t2 * f4_E1[1] + t1 * f4_E2[1]),
			s * (-t2 * f4_E1[2] + t1 * f4_E2[2])
		};

		GMVec3 tangentVector = FastNormalize(GMVec3(tangents[0], tangents[1], tangents[2]));
		GMVec3 bitangentVector = FastNormalize(GMVec3(bitangents[0], bitangents[1], bitangents[2]));
		GMFloat4 f4_tangentVector, f4_bitangentVector;
		tangentVector.loadFloat4(f4_tangentVector);
		bitangentVector.loadFloat4(f4_bitangentVector);

		currentVertex.tangents[0] = f4_tangentVector[0];
		currentVertex.tangents[1] = f4_tangentVector[1];
		currentVertex.tangents[2] = f4_tangentVector[2];

		currentVertex.bitangents[0] = f4_bitangentVector[0];
		currentVertex.bitangents[1] = f4_bitangentVector[1];
		currentVertex.bitangents[2] = f4_bitangentVector[2];
	}
}

bool GMPart::calculateNormals(GMModelDrawMode drawMode, GMTopologyMode topologyMode, GMS_FrontFace frontFace)
{
	D(d);
	// 一定要是顶点模式，而不是索引模式，不然算出来的法向量没有意义
	if (topologyMode == GMTopologyMode::Triangles && drawMode == GMModelDrawMode::Vertex)
	{
		// 顶点成三角形拓扑，每个面的法线为它们的Cross Production
		for (GMsize_t i = 0; i < d->vertices.size(); i += 3)
		{
			GMVertex& vertex0 = d->vertices.at(i);
			GMVertex& vertex1 = d->vertices.at(i + 1);
			GMVertex& vertex2 = d->vertices.at(i + 2);
			GMVec3 p0(vertex0.positions[0], vertex0.positions[1], vertex0.positions[2]);
			GMVec3 p1(vertex1.positions[0], vertex1.positions[1], vertex1.positions[2]);
			GMVec3 p2(vertex2.positions[0], vertex2.positions[1], vertex2.positions[2]);
			GMVec3 v0 = p1 - p0, v1 = p2 - p0;
			GMVec3 normal = Normalize(Cross(v0, v1));
			if (frontFace == GMS_FrontFace::CounterClosewise)
				normal = -normal;
			CopyToArray(normal, &vertex0.normals[0]);
			CopyToArray(normal, &vertex1.normals[0]);
			CopyToArray(normal, &vertex2.normals[0]);
		}
		return true;
	}

	gm_warning(gm_dbg_wrap("topologyMode is not supported."));
	return false;
}
//////////////////////////////////////////////////////////////////////////

GM_PRIVATE_OBJECT_ALIGNED(GMNode)
{
	GMString name;
	GMNodePtr parent = nullptr;
	Vector<GMNodePtr> children;
	GMMat4 transformToParent;
	GMMat4 globalTransform;
	Vector<GMuint32> modelIndices;
};

GM_DEFINE_PROPERTY(GMNode, GMString, Name, name)
GM_DEFINE_PROPERTY(GMNode, GMNodePtr, Parent, parent)
GM_DEFINE_PROPERTY(GMNode, Vector<GMNodePtr>, Children, children)
GM_DEFINE_PROPERTY(GMNode, Vector<GMuint32>, ModelIndices, modelIndices)
GM_DEFINE_PROPERTY(GMNode, GMMat4, TransformToParent, transformToParent)
GM_DEFINE_PROPERTY(GMNode, GMMat4, GlobalTransform, globalTransform)

GMNode::GMNode()
{
	GM_CREATE_DATA(GMNode);
}

GMNode::~GMNode()
{
	for (auto& child : getChildren())
	{
		GM_delete(child);
	}
}

GM_PRIVATE_OBJECT_UNALIGNED(GMSkeletalBones)
{
	AlignedVector<GMSkeletalBone> bones;
	Map<GMString, GMsize_t> boneNameIndexMap;
};

GM_DEFINE_PROPERTY(GMSkeletalBones, AlignedVector<GMSkeletalBone>, Bones, bones)
GM_DEFINE_PROPERTY(GMSkeletalBones, GMSkeletalBones::BoneNameIndexMap_t, BoneNameIndexMap, boneNameIndexMap)
GMSkeletalBones::GMSkeletalBones()
{
	GM_CREATE_DATA(GMSkeletalBones);
}

GMSkeletalBones::GMSkeletalBones(const GMSkeletalBones& rhs)
{
	*this = rhs;
}

GMSkeletalBones::GMSkeletalBones(GMSkeletalBones&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMSkeletalBones& GMSkeletalBones::operator=(const GMSkeletalBones& rhs)
{
	GM_COPY(rhs);
	return *this;
}

GMSkeletalBones& GMSkeletalBones::operator=(GMSkeletalBones&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMSkeleton)
{
	GMSkeletalBones bones;
};

GM_DEFINE_PROPERTY(GMSkeleton, GMSkeletalBones, Bones, bones)
GMSkeleton::GMSkeleton()
{
	GM_CREATE_DATA(GMSkeleton);
}

GM_PRIVATE_OBJECT_UNALIGNED(GMSkeletalAnimations)
{
	AlignedVector<GMNodeAnimation> animations;
};

GMSkeletalAnimations::GMSkeletalAnimations()
{
	GM_CREATE_DATA(GMSkeletalAnimations);
}

GMNodeAnimation* GMSkeletalAnimations::getAnimation(GMsize_t index) GM_NOEXCEPT
{
	D(d);
	return &d->animations[index];
}

GMsize_t GMSkeletalAnimations::getAnimationCount()
{
	D(d);
	return d->animations.size();
}

//////////////////////////////////////////////////////////////////////////

END_NS
