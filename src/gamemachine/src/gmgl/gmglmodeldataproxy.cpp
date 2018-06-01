#include "stdafx.h"
#include <GL/glew.h>
#include "gmglmodeldataproxy.h"
#include "shader_constants.h"
#include "gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"

#define FLOAT_OFFSET(i) ((void*)(sizeof(gm::GMfloat) * i))

GMGLModelDataProxy::GMGLModelDataProxy(const GMContext* context, GMModel* objs)
	: GMModelDataProxy(context, objs)
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(context->engine);
}

void GMGLModelDataProxy::transfer()
{
	D(d);
	if (d->inited)
		return;

	GMModel* model = getModel();
	if (!model->isNeedTransfer())
		return;

	prepareTangentSpace();

	GMModelBufferData bufferData;
	Vector<GMVertex> packedVertices;
	// 把数据打入顶点数组
	packVertices(packedVertices);

	GMsize_t verticeCount = 0;

	GLuint vao;
	glGenVertexArrays(1, &vao);
	bufferData.arrayId = vao;
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	bufferData.vertexBufferId = vbo;

	GLenum usage = model->getUsageHint() == GMUsageHint::StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	glBindBuffer(GL_ARRAY_BUFFER, bufferData.vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GMVertex) * packedVertices.size(), packedVertices.data(), usage);

	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Position),	GMVertex::PositionDimension,	GL_FLOAT, GL_FALSE, sizeof(GMVertex), 0);
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Normal),		GMVertex::NormalDimension,		GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(3));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Texcoord),	GMVertex::TexcoordDimension,	GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(6));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Tangent),		GMVertex::TangentDimension,		GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(8));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Bitangent),	GMVertex::BitangentDimension,	GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(11));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Lightmap),	GMVertex::LightmapDimension,	GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(14));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Color),		GMVertex::ColorDimension,		GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(16));

	GM_FOREACH_ENUM_CLASS(type, GMVertexDataType::Position, GMVertexDataType::EndOfVertexDataType)
	{
		glEnableVertexAttribArray(gmVertexIndex(type));
	}
	
	if (model->getDrawMode() == GMModelDrawMode::Index)
	{
		Vector<GMuint> packedIndices;
		// 把数据打入顶点数组
		packIndices(packedIndices);

		glGenBuffers(1, &bufferData.indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferData.indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GMuint) * packedIndices.size(), packedIndices.data(), GL_STATIC_DRAW);

		verticeCount = packedIndices.size();
	}
	else
	{
		verticeCount = packedVertices.size();
	}

	glBindVertexArray(0);

	for (auto& mesh : model->getMeshes())
	{
		mesh->clear();
	}

	GMModelBuffer* modelBuffer = new GMModelBuffer();
	modelBuffer->setData(bufferData);

	model->setVerticesCount(verticeCount);
	model->setModelBuffer(modelBuffer);
	modelBuffer->releaseRef();
	d->inited = true;
	model->needNotTransferAnymore();
}

void GMGLModelDataProxy::dispose(GMModelBuffer* md)
{
	D(d);
	GLuint vao[1] = { md->getMeshBuffer().arrayId },
		vbo[1] = { md->getMeshBuffer().vertexBufferId };

	glDeleteVertexArrays(1, vao);
	glDeleteBuffers(1, vbo);
	d->inited = false;
}

void GMGLModelDataProxy::beginUpdateBuffer()
{
	GMModel* model = getModel();
	GM_ASSERT(model);
	glBindVertexArray(model->getModelBuffer()->getMeshBuffer().arrayId);
	glBindBuffer(GL_ARRAY_BUFFER, model->getModelBuffer()->getMeshBuffer().vertexBufferId);
}

void GMGLModelDataProxy::endUpdateBuffer()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void* GMGLModelDataProxy::getBuffer()
{
	return glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
}