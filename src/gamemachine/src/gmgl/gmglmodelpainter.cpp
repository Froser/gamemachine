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

	GMMesh* mesh = model->getMesh();
	GLenum usage = model->getUsageHint() == GMUsageHint::StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

	mesh->calculateTangentSpace();

	GMMeshBuffer meshBuffer;

	GM_BEGIN_CHECK_GL_ERROR
	GLuint vao;
	glGenVertexArrays(1, &vao);
	meshBuffer.arrayId = vao;
	glBindVertexArray(vao);

	Vector<GMVertex> packedData;
	// 把数据打入顶点数组
	packData(packedData);
	GLuint dataSize = sizeof(GMVertex) * packedData.size();

	GLuint vbo;
	glGenBuffers(1, &vbo);
	meshBuffer.bufferId = vbo;

	glBindBuffer(GL_ARRAY_BUFFER, meshBuffer.bufferId);
	glBufferData(GL_ARRAY_BUFFER, dataSize, packedData.data(), usage);

	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Position),	GMModel::PositionDimension,	 GL_FLOAT, GL_FALSE, sizeof(GMVertex), 0);
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Normal),		GMModel::NormalDimension,	 GL_FLOAT, GL_TRUE,  sizeof(GMVertex), FLOAT_OFFSET(3));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::UV),			GMModel::TexcoordDimension,	 GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(6));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Tangent),		GMModel::TangentDimension,	 GL_FLOAT, GL_TRUE,  sizeof(GMVertex), FLOAT_OFFSET(8));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Bitangent),	GMModel::BitangentDimension, GL_FLOAT, GL_TRUE,  sizeof(GMVertex), FLOAT_OFFSET(11));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Lightmap),	GMModel::LightmapDimension,	 GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(14));
	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Color),		GMModel::TextureDimension,	 GL_FLOAT, GL_FALSE, sizeof(GMVertex), FLOAT_OFFSET(16));

	GM_FOREACH_ENUM_CLASS(type, GMVertexDataType::Position, GMVertexDataType::EndOfVertexDataType)
	{
		glEnableVertexAttribArray(gmVertexIndex(type));
	}

	glBindVertexArray(0);

	mesh->clear_positions_and_save_byte_size();
	mesh->clear_normals_and_save_byte_size();
	mesh->clear_texcoords_and_save_byte_size();
	mesh->clear_tangents_and_save_byte_size();
	mesh->clear_bitangents_and_save_byte_size();
	mesh->clear_lightmaps_and_save_byte_size();
	mesh->clear_colors_and_save_byte_size();

	GM_END_CHECK_GL_ERROR

	mesh->setMeshBuffer(meshBuffer);
	d->inited = true;
	model->needNotTransferAnymore();
}

void GMGLModelPainter::draw(const GMGameObject* parent)
{
	D(d);
	GMModel* model = getModel();
	IRenderer* renderer = d->engine->getRenderer(model->getType());
	renderer->beginModel(model, parent);

	GMMesh* mesh = model->getMesh();
	glBindVertexArray(mesh->getMeshBuffer().arrayId);
	for (auto component : mesh->getComponents())
	{
		GMShader& shader = component->getShader();
		if (shader.getNodraw())
			continue;

		draw(renderer, component, mesh);
	}
	glBindVertexArray(0);

	renderer->endModel();
}

void GMGLModelPainter::dispose(GMMeshData* md)
{
	D(d);
	GLuint vao[1] = { md->getMeshBuffer().arrayId },
		vbo[1] = { md->getMeshBuffer().bufferId };

	GM_BEGIN_CHECK_GL_ERROR
	glDeleteVertexArrays(1, vao);
	GM_END_CHECK_GL_ERROR

	GM_BEGIN_CHECK_GL_ERROR
	glDeleteBuffers(1, vbo);
	GM_END_CHECK_GL_ERROR

	d->inited = false;
}

void GMGLModelPainter::beginUpdateBuffer(GMMesh* mesh)
{
	glBindVertexArray(mesh->getMeshBuffer().arrayId);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->getMeshBuffer().bufferId);
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

void GMGLModelPainter::draw(IRenderer* renderer, GMComponent* component, GMMesh* mesh)
{
	D(d);
	d->engine->checkDrawingState();
	renderer->draw(this, component, mesh);
}