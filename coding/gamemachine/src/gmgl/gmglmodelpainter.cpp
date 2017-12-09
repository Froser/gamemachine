#include "stdafx.h"
#include "gmglmodelpainter.h"
#include "shader_constants.h"
#include "gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"

namespace
{
	GLenum getMode(GMMesh* obj)
	{
		switch (obj->getArrangementMode())
		{
		case GMArrangementMode::Triangle_Fan:
			return GL_TRIANGLE_FAN;
		case GMArrangementMode::Triangle_Strip:
			return GL_TRIANGLE_STRIP;
		case GMArrangementMode::Triangles:
			return GL_TRIANGLES;
		case GMArrangementMode::Lines:
			return GL_LINE_LOOP;
		default:
			GM_ASSERT(false);
			return GL_TRIANGLE_FAN;
		}
	}
}

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
	GMMesh* mesh = model->getMesh();
	GLenum usage = model->getUsageHint() == GMUsageHint::StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

	mesh->calculateTangentSpace();

	GM_BEGIN_CHECK_GL_ERROR
	GLuint vao;
	glGenVertexArrays(1, &vao);
	mesh->setArrayId(vao);

	glBindVertexArray(mesh->getArrayId());
	GLuint positionSize		= mesh->isDataDisabled(GMVertexDataType::Position)		? 0 : sizeof(GMModel::DataType) * mesh->positions().size();
	GLuint normalSize		= mesh->isDataDisabled(GMVertexDataType::Normal)		? 0 : sizeof(GMModel::DataType) * mesh->normals().size();
	GLuint uvSize			= mesh->isDataDisabled(GMVertexDataType::UV)			? 0 : sizeof(GMModel::DataType) * mesh->uvs().size();
	GLuint tangentSize		= mesh->isDataDisabled(GMVertexDataType::Tangent)		? 0 : sizeof(GMModel::DataType) * mesh->tangents().size();
	GLuint bitangentSize	= mesh->isDataDisabled(GMVertexDataType::Bitangent)		? 0 : sizeof(GMModel::DataType) * mesh->bitangents().size();
	GLuint lightmapSize		= mesh->isDataDisabled(GMVertexDataType::Lightmap)		? 0 : sizeof(GMModel::DataType) * mesh->lightmaps().size();
	GLuint colorSize		= mesh->isDataDisabled(GMVertexDataType::Color)			? 0 : sizeof(GMModel::DataType) * mesh->colors().size();

	GLuint vbo;
	glGenBuffers(1, &vbo);
	mesh->setBufferId(vbo);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->getBufferId());
	glBufferData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize + colorSize, NULL, usage);
	IF_ENABLED(mesh, GMVertexDataType::Position)	glBufferSubData(GL_ARRAY_BUFFER, 0																				, positionSize	, mesh->positions().data());
	IF_ENABLED(mesh, GMVertexDataType::Normal)		glBufferSubData(GL_ARRAY_BUFFER, positionSize																	, normalSize	, mesh->normals().data());
	IF_ENABLED(mesh, GMVertexDataType::UV)			glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize														, uvSize		, mesh->uvs().data());
	IF_ENABLED(mesh, GMVertexDataType::Tangent)		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize												, tangentSize	, mesh->tangents().data());
	IF_ENABLED(mesh, GMVertexDataType::Bitangent)	glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize								, bitangentSize	, mesh->bitangents().data());
	IF_ENABLED(mesh, GMVertexDataType::Lightmap)	glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize				, lightmapSize	, mesh->lightmaps().data());
	IF_ENABLED(mesh, GMVertexDataType::Color)		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize, colorSize		, mesh->colors().data());

	IF_ENABLED(mesh, GMVertexDataType::Position)	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Position),	4,  GL_FLOAT, GL_FALSE, 0, 0);
	IF_ENABLED(mesh, GMVertexDataType::Normal)		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Normal),		4,  GL_FLOAT, GL_TRUE,  0, (void*)positionSize);
	IF_ENABLED(mesh, GMVertexDataType::UV)			glVertexAttribPointer(gmVertexIndex(GMVertexDataType::UV),			2,  GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize));
	IF_ENABLED(mesh, GMVertexDataType::Tangent)		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Tangent),		4,  GL_FLOAT, GL_TRUE,  0, (void*)(positionSize + normalSize + uvSize));
	IF_ENABLED(mesh, GMVertexDataType::Bitangent)	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Bitangent),	4,  GL_FLOAT, GL_TRUE,  0, (void*)(positionSize + normalSize + uvSize + tangentSize));
	IF_ENABLED(mesh, GMVertexDataType::Lightmap)	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Lightmap),	2,  GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize + uvSize + tangentSize + bitangentSize));
	IF_ENABLED(mesh, GMVertexDataType::Color)		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Color),		4,  GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize));

	GM_FOREACH_ENUM_CLASS(type, GMVertexDataType::Position, GMVertexDataType::EndOfVertexDataType)
	{
		if (!mesh->isDataDisabled(type))
			glEnableVertexAttribArray(gmVertexIndex(type));
	}

	glBindVertexArray(0);

	IF_ENABLED(mesh, GMVertexDataType::Position)	mesh->clear_positions_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Normal)		mesh->clear_normals_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::UV)			mesh->clear_uvs_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Tangent)		mesh->clear_tangents_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Bitangent)	mesh->clear_bitangents_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Lightmap)	mesh->clear_lightmaps_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Color)		mesh->clear_colors_and_save_byte_size();

	GM_END_CHECK_GL_ERROR

	d->inited = true;
}

void GMGLModelPainter::draw(const GMGameObject* parent)
{
	D(d);
	GMModel* model = getModel();
	IRenderer* renderer = d->engine->getRenderer(model->getType());
	renderer->beginModel(model, parent);

	GMMesh* mesh = model->getMesh();
	glBindVertexArray(mesh->getArrayId());
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

void GMGLModelPainter::dispose()
{
	D(d);
	GMMesh* mesh = getModel()->getMesh();
	GLuint vao[1] = { mesh->getArrayId() },
		vbo[1] = { mesh->getBufferId() };

	glDeleteVertexArrays(1, vao);
	glDeleteBuffers(1, vbo);

	d->inited = false;
}

void GMGLModelPainter::beginUpdateBuffer(GMMesh* mesh)
{
	glBindVertexArray(mesh->getArrayId());
	glBindBuffer(GL_ARRAY_BUFFER, mesh->getBufferId());
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

	GM_BEGIN_CHECK_GL_ERROR
	renderer->beginComponent(component);
	GM_END_CHECK_GL_ERROR

	GM_BEGIN_CHECK_GL_ERROR
	GLenum mode = GMGetDebugState(POLYGON_LINE_MODE) ? GL_LINE_LOOP : getMode(mesh);
	glMultiDrawArrays(mode, component->getOffsetPtr(), component->getPrimitiveVerticesCountPtr(), component->getPrimitiveCount());
	GM_END_CHECK_GL_ERROR

	renderer->endComponent();
}