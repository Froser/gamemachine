#include "stdafx.h"
#include "gmglobjectpainter.h"
#include "shader_constants.h"
#include "gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"

#define IF_ENABLED(mesh, type) if (!mesh->isDataDisabled(type))

static GLenum getMode(GMMesh* obj)
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
		ASSERT(false);
		return GL_TRIANGLE_FAN;
	}
}

GMGLObjectPainter::GMGLObjectPainter(IGraphicEngine* engine, Object* objs)
	: GMObjectPainter(objs)
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(engine);
}

void GMGLObjectPainter::transfer()
{
	D(d);
	if (d->inited)
		return;

	Object* obj = getObject();
	GLenum usage = obj->getHint() == GMUsageHint::StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	BEGIN_FOREACH_OBJ(obj, mesh)
	{
		mesh->calculateTangentSpace();

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		mesh->setArrayId(vao);

		GLuint positionSize		= mesh->isDataDisabled(GMVertexDataType::Position)		? 0 : sizeof(Object::DataType) * mesh->positions().size();
		GLuint normalSize		= mesh->isDataDisabled(GMVertexDataType::Normal)		? 0 : sizeof(Object::DataType) * mesh->normals().size();
		GLuint uvSize			= mesh->isDataDisabled(GMVertexDataType::UV)			? 0 : sizeof(Object::DataType) * mesh->uvs().size();
		GLuint tangentSize		= mesh->isDataDisabled(GMVertexDataType::Tangent)		? 0 : sizeof(Object::DataType) * mesh->tangents().size();
		GLuint bitangentSize	= mesh->isDataDisabled(GMVertexDataType::Bitangent)		? 0 : sizeof(Object::DataType) * mesh->bitangents().size();
		GLuint lightmapSize		= mesh->isDataDisabled(GMVertexDataType::Lightmap)		? 0 : sizeof(Object::DataType) * mesh->lightmaps().size();
		GLuint colorSize		= mesh->isDataDisabled(GMVertexDataType::Color)			? 0 : sizeof(Object::DataType) * mesh->colors().size();

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize + colorSize, NULL, usage);
		IF_ENABLED(mesh, GMVertexDataType::Position)	glBufferSubData(GL_ARRAY_BUFFER, 0																				, positionSize	, mesh->positions().data());
		IF_ENABLED(mesh, GMVertexDataType::Normal)		glBufferSubData(GL_ARRAY_BUFFER, positionSize																	, normalSize	, mesh->normals().data());
		IF_ENABLED(mesh, GMVertexDataType::UV)			glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize														, uvSize		, mesh->uvs().data());
		IF_ENABLED(mesh, GMVertexDataType::Tangent)		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize												, tangentSize	, mesh->tangents().data());
		IF_ENABLED(mesh, GMVertexDataType::Bitangent)	glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize								, bitangentSize	, mesh->bitangents().data());
		IF_ENABLED(mesh, GMVertexDataType::Lightmap)	glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize				, lightmapSize	, mesh->lightmaps().data());
		IF_ENABLED(mesh, GMVertexDataType::Color)		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize, colorSize		, mesh->colors().data());
		mesh->setBufferId(vbo);

		IF_ENABLED(mesh, GMVertexDataType::Position)	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Position),	4,  GL_FLOAT, GL_FALSE, 0, 0);
		IF_ENABLED(mesh, GMVertexDataType::Normal)		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Normal),		4,  GL_FLOAT, GL_TRUE,  0, (void*)positionSize);
		IF_ENABLED(mesh, GMVertexDataType::UV)			glVertexAttribPointer(gmVertexIndex(GMVertexDataType::UV),			2,  GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize));
		IF_ENABLED(mesh, GMVertexDataType::Tangent)		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Tangent),		4,  GL_FLOAT, GL_TRUE,  0, (void*)(positionSize + normalSize + uvSize));
		IF_ENABLED(mesh, GMVertexDataType::Bitangent)	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Bitangent),	4,  GL_FLOAT, GL_TRUE,  0, (void*)(positionSize + normalSize + uvSize + tangentSize));
		IF_ENABLED(mesh, GMVertexDataType::Lightmap)	glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Lightmap),	2,  GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize + uvSize + tangentSize + bitangentSize));
		IF_ENABLED(mesh, GMVertexDataType::Color)		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Color),		4,  GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize));

		GM_FOREACH_ENUM_CLASS(type, GMVertexDataType, GMVertexDataType::Position, GMVertexDataType::EndOfVertexDataType)
		{
			if (!mesh->isDataDisabled(type))
				glEnableVertexAttribArray(gmVertexIndex(type));
		}

		glBindVertexArray(0);

		mesh->clear_positions_and_save_byte_size();
		mesh->clear_normals_and_save_byte_size();
		mesh->clear_uvs_and_save_byte_size();
		mesh->clear_tangents_and_save_byte_size();
		mesh->clear_bitangents_and_save_byte_size();
		mesh->clear_lightmaps_and_save_byte_size();
		mesh->clear_colors_and_save_byte_size();
	}
	END_FOREACH_OBJ

	d->inited = true;
}

void GMGLObjectPainter::draw(GMfloat* modelTransform)
{
	D(d);
	Object* obj = getObject();

	BEGIN_FOREACH_OBJ(obj, mesh)
	{
		IRender* render = d->engine->getRender(mesh->getType());
		render->begin(d->engine, mesh, modelTransform);

		glBindVertexArray(mesh->getArrayId());
		for (auto component : mesh->getComponents())
		{
			Shader& shader = component->getShader();
			if (shader.getNodraw())
				continue;

			draw(render, shader, component, mesh, true);
			if (shader.getDrawBorder())
				draw(render, shader, component, mesh, false);
		}
		glBindVertexArray(0);

		render->end();
	}
	END_FOREACH_OBJ
}

void GMGLObjectPainter::dispose()
{
	D(d);
	Object* obj = getObject();

	BEGIN_FOREACH_OBJ(obj, mesh)
	{
		GLuint vao[1] = { mesh->getArrayId() },
			vbo[1] = { mesh->getBufferId() };

		glDeleteVertexArrays(1, vao);
		glDeleteBuffers(1, vbo);
	}
	END_FOREACH_OBJ

	d->inited = false;
}

void GMGLObjectPainter::beginUpdateBuffer(GMMesh* mesh)
{
	glBindVertexArray(mesh->getArrayId());
}

void GMGLObjectPainter::endUpdateBuffer()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindVertexArray(0);
}

void* GMGLObjectPainter::getBuffer()
{
	return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void GMGLObjectPainter::draw(IRender* render, Shader& shader, Component* component, GMMesh* mesh, bool fill)
{
	GLenum mode = GMGetBuiltIn(POLYGON_LINE_MODE) ? GL_LINE_LOOP : getMode(mesh);
	if (fill)
		render->beginShader(shader, GMDrawMode::Fill);
	else
		render->beginShader(shader, GMDrawMode::Line);

	glMultiDrawArrays(mode, component->getOffsetPtr(), component->getPrimitiveVerticesCountPtr(), component->getPrimitiveCount());
	render->endShader();
}