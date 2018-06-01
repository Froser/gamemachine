#include "stdafx.h"
#include "gmmodel.h"
#include <linearmath.h>
#include <algorithm>
#include <iterator>
#include "foundation/gamemachine.h"

#ifdef max
#undef max
#endif

#define TO_VEC3(i) GMVec3((i)[0], (i)[1], (i)[2])
#define TO_VEC2(i) GMVec2((i)[0], (i)[1])

const GMContext* GMModelDataProxy::getContext()
{
	D(d);
	return d->context;
}

void GMModelDataProxy::prepareTangentSpace()
{
	GMModel* model = getModel();
	if (model->getDrawMode() == GMModelDrawMode::Vertex)
	{
		for (auto& mesh : model->getMeshes())
		{
			if (model->getShader().getTextureList().getTextureSampler(GMTextureType::NormalMap).getFrameCount() > 0)
				mesh->calculateTangentSpace(model->getPrimitiveTopologyMode());
		}
	}
	else
	{
		for (auto& mesh : model->getMeshes())
		{
			mesh->invalidateTangentSpace();
		}
	}
}

void GMModelDataProxy::packVertices(Vector<GMVertex>& vertices)
{
	GMModel* model = getModel();
	GMMeshes& meshes = model->getMeshes();
	GMuint offset = 0;
	for (auto& mesh : meshes)
	{
		for (auto& vertex : mesh->vertices())
		{
			vertices.push_back(vertex);
		}
	}
}

void GMModelDataProxy::packIndices(Vector<GMuint>& indices)
{
	GMModel* model = getModel();
	GMMeshes& meshes = model->getMeshes();
	GMuint offset = 0;
	for (auto& mesh : meshes)
	{
		for (GMuint index : mesh->indices())
		{
			indices.push_back(index + offset);
		}

		// 每个Mesh按照自己的坐标排序，因此每个Mesh都应该在总缓存里面加上偏移
		offset += (GMuint)mesh->vertices().size();
		GM_ASSERT(offset < std::numeric_limits<GMuint>::max());
	}
}

GMModel::GMModel()
{
	D(d);
	d->modelBuffer = new GMModelBuffer();
}

GMModel::GMModel(GMModel& model)
{
	D(d);
	setModelBuffer(model.getModelBuffer());
	setPrimitiveTopologyMode(model.getPrimitiveTopologyMode());
	setShader(model.getShader());
	setVerticesCount(model.getVerticesCount());
	if (!model.getModelDataProxy())
		GM.createModelDataProxyAndTransfer(getModelDataProxy()->getContext(), &model);
	needNotTransferAnymore();
}

GMModel::~GMModel()
{
	D(d);
	releaseModelBuffer();
	for (auto& mesh : d->meshes)
	{
		GM_delete(mesh);
	}
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

GMMesh::GMMesh(GMModel* parent)
{
	parent->addMesh(this);
}

void GMMesh::clear()
{
	D(d);
	GMClearSTLContainer(d->vertices);
	GMClearSTLContainer(d->indices);
}

void GMMesh::vertex(const GMVertex& v)
{
	D(d);
	d->vertices.push_back(v);
}

void GMMesh::index(GMuint index)
{
	D(d);
	d->indices.push_back(index);
}

void GMMesh::invalidateTangentSpace()
{
	D(d);
	for (auto& vertex : d->vertices)
	{
		vertex.tangents[0] = vertex.bitangents[0] = 
		vertex.tangents[1] = vertex.bitangents[1] = 
		vertex.tangents[2] = vertex.bitangents[2] = InvalidTangentSpace;
	}
}

void GMMesh::calculateTangentSpace(GMTopologyMode topologyMode)
{
	D(d);
	if (topologyMode == GMTopologyMode::Lines)
		return;

	for (GMuint i = 0; i < d->vertices.size(); i++)
	{
		GMVec3 e0, e1, e2;
		GMVec2 uv0, uv1, uv2;
		GMVertex& currentVertex = d->vertices[i];
		if (topologyMode == GMTopologyMode::Triangles)
		{
			GMuint startIndex = i / 3 * 3;
			GMuint vertexIndices[3] = { startIndex, startIndex + 1, startIndex + 2 };
			GMuint indexInTriangle = i % 3;
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
				GMuint vertexIndices[3] = { 0, 1, 2 };
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