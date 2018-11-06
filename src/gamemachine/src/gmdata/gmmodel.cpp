#include "stdafx.h"
#include "gmmodel.h"
#include <linearmath.h>
#include <algorithm>
#include <iterator>
#include "foundation/gamemachine.h"

#define TO_VEC3(i) GMVec3((i)[0], (i)[1], (i)[2])
#define TO_VEC2(i) GMVec2((i)[0], (i)[1])

void GMScene::push_back(GMModelAsset model)
{
	D(d);
	d->models.push_back(model);
}

void GMScene::swap(GMScene* scene)
{
	D(d);
	D_OF(d_rhs, scene);
	using std::swap;
	d->models.swap(d_rhs->models);
	d->skeletalRoot.swap(d_rhs->skeletalRoot);
}

const IRenderContext* GMModelDataProxy::getContext()
{
	D(d);
	return d->context;
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
	GMParts& meshes = model->getParts();
	GMuint32 offset = 0;
	for (auto& part : meshes)
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
	GMParts& meshes = model->getParts();
	GMuint32 offset = 0;
	for (auto& part : meshes)
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
		d->context->getEngine()->createModelDataProxy(d->context, parent, true);
		// 参考 GMModel::GMModel(GMModelAsset parentAsset)
		d->model->setModelBuffer(parent->getModelBuffer());
		d->model->setPrimitiveTopologyMode(parent->getPrimitiveTopologyMode());
		d->model->setVerticesCount(parent->getVerticesCount());
	}
}

GMModel::GMModel()
{
	D(d);
	d->modelBuffer = new GMModelBuffer();
}

GMModel::GMModel(GMModelAsset parentAsset)
{
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

GMModelBuffer::GMModelBuffer()
{
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

GMPart::GMPart(GMModel* parent)
{
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

bool GMPart::calculateNormals(GMTopologyMode topologyMode, GMS_FrontFace frontFace)
{
	D(d);
	// 一定要是顶点模式，而不是索引模式，不然算出来的法向量没有意义
	if (topologyMode == GMTopologyMode::Triangles)
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