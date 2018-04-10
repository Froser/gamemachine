#include "stdafx.h"
#include <GL/glew.h>
#include "gmglmodelpainter.h"
#include "shader_constants.h"
#include "gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"

#define FLOAT_OFFSET(i) ((void*)(sizeof(gm::GMfloat) * i))

GMGLModelPainter::GMGLModelPainter(IGraphicEngine* engine, GMModel* objs)
	: GMModelPainter(objs)
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(engine);
}

void GMGLModelPainter::transfer()
{
	D(d);
	if (d->inited)
		return;

	GMModel* model = getModel();
	if (!model->isNeedTransfer())
		return;

	GLenum usage = model->getUsageHint() == GMUsageHint::StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

	for (auto& mesh : model->getMeshes())
	{
		mesh->calculateTangentSpace();
	}

	GMModelBufferData bufferData;
	Vector<GMVertex> packedVertices;
	// 把数据打入顶点数组
	packData(packedVertices);

	GMuint verticeCount = 0;

	GM_BEGIN_CHECK_GL_ERROR
	GLuint vao;
	glGenVertexArrays(1, &vao);
	bufferData.arrayId = vao;
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	bufferData.vertexBufferId = vbo;

	glBindBuffer(GL_ARRAY_BUFFER, bufferData.vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GMVertex) * packedVertices.size(), packedVertices.data(), usage);

	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Position),	GMVertex::PositionDimension,	GL_FLOAT, GL_FALSE, sizeof(GMVertex), 0);
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Normal),		GMVertex::NormalDimension,		GL_FLOAT, GL_TRUE,  sizeof(GMVertex), FLOAT_OFFSET(3));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::UV),			GMVertex::TexcoordDimension,	GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(6));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Tangent),		GMVertex::TangentDimension,		GL_FLOAT, GL_TRUE,  sizeof(GMVertex), FLOAT_OFFSET(8));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Bitangent),	GMVertex::BitangentDimension,	GL_FLOAT, GL_TRUE,  sizeof(GMVertex), FLOAT_OFFSET(11));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Lightmap),	GMVertex::LightmapDimension,	GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(14));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Color),		GMVertex::TextureDimension,		GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(16));

	GM_FOREACH_ENUM_CLASS(type, GMVertexDataType::Position, GMVertexDataType::EndOfVertexDataType)
	{
		glEnableVertexAttribArray(gmVertexIndex(type));
	}
	
	if (model->getBufferType() == GMModelBufferType::IndexBuffer)
	{
		Vector<GMVertex> packedIndices;
		// 把数据打入顶点数组
		packData(packedIndices);

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

	GM_END_CHECK_GL_ERROR

	GMModelBuffer* modelBuffer = new GMModelBuffer();
	modelBuffer->setData(bufferData);

	model->setVerticesCount(verticeCount);
	model->setModelBuffer(modelBuffer);
	modelBuffer->releaseRef();
	d->inited = true;
	model->needNotTransferAnymore();
}

void GMGLModelPainter::draw(const GMGameObject* parent)
{
	D(d);
	GMModel* model = getModel();
	IRenderer* renderer = d->engine->getRenderer(model->getType());
	renderer->beginModel(model, parent);

	if (model->getShader().getDiscard())
		return;

	glBindVertexArray(model->getBuffer()->arrayId);
	draw(renderer, model);
	glBindVertexArray(0);

	renderer->endModel();
}

void GMGLModelPainter::dispose(GMModelBuffer* md)
{
	D(d);
	GLuint vao[1] = { md->getMeshBuffer().arrayId },
		vbo[1] = { md->getMeshBuffer().vertexBufferId };

	GM_BEGIN_CHECK_GL_ERROR
	glDeleteVertexArrays(1, vao);
	GM_END_CHECK_GL_ERROR

	GM_BEGIN_CHECK_GL_ERROR
	glDeleteBuffers(1, vbo);
	GM_END_CHECK_GL_ERROR

	d->inited = false;
}

void GMGLModelPainter::beginUpdateBuffer(GMModel* model)
{
	glBindVertexArray(model->getBuffer()->arrayId);
	glBindBuffer(GL_ARRAY_BUFFER, model->getBuffer()->vertexBufferId);
}

void GMGLModelPainter::endUpdateBuffer()
{
	GM_BEGIN_CHECK_GL_ERROR
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	GM_END_CHECK_GL_ERROR
}

void* GMGLModelPainter::getBuffer()
{
	return glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
}

void GMGLModelPainter::draw(IRenderer* renderer, GMModel* model)
{
	D(d);
	d->engine->checkDrawingState();
	renderer->draw(this, model);
}