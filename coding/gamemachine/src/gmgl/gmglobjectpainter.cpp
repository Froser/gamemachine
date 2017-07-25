#include "stdafx.h"
#include "gmglobjectpainter.h"
#include "shader_constants.h"
#include "gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"

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

		GLuint positionSize = sizeof(Object::DataType) * mesh->positions().size();
		GLuint normalSize = sizeof(Object::DataType) * mesh->normals().size();
		GLuint uvSize = sizeof(Object::DataType) * mesh->uvs().size();
		GLuint tangentSize = sizeof(Object::DataType) * mesh->tangents().size();
		GLuint bitangentSize = sizeof(Object::DataType) * mesh->bitangents().size();
		GLuint lightmapSize = sizeof(Object::DataType) * mesh->lightmaps().size();
		GLuint colorSize = sizeof(Object::DataType) * mesh->colors().size();

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize + colorSize, NULL, usage);
		glBufferSubData(GL_ARRAY_BUFFER, 0																		, positionSize, mesh->positions().data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize																, normalSize, mesh->normals().data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize													, uvSize, mesh->uvs().data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize											, tangentSize, mesh->tangents().data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize							, bitangentSize, mesh->bitangents().data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize			, lightmapSize, mesh->lightmaps().data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize + normalSize + uvSize + tangentSize + bitangentSize + colorSize, lightmapSize, mesh->colors().data());
		mesh->setBufferId(vbo);

		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Position),		4, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Normal),		4, GL_FLOAT, GL_TRUE,  0, (void*)positionSize);
		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::UV),			2, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize));
		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Tangent),		4, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize + uvSize));
		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Bitangent),	4, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize + uvSize + tangentSize));
		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Lightmap),	2, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize + uvSize + tangentSize + bitangentSize));
		glVertexAttribPointer(gmVertexIndex(GMVertexDataType::Color),		4, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + normalSize + uvSize + tangentSize + bitangentSize + colorSize));
		glEnableVertexAttribArray(gmVertexIndex(GMVertexDataType::Position));
		glEnableVertexAttribArray(gmVertexIndex(GMVertexDataType::Normal));
		glEnableVertexAttribArray(gmVertexIndex(GMVertexDataType::UV));
		glEnableVertexAttribArray(gmVertexIndex(GMVertexDataType::Tangent));
		glEnableVertexAttribArray(gmVertexIndex(GMVertexDataType::Bitangent));
		glEnableVertexAttribArray(gmVertexIndex(GMVertexDataType::Lightmap));
		glEnableVertexAttribArray(gmVertexIndex(GMVertexDataType::Color));

		glBindVertexArray(0);

		mesh->positions().clear();
		mesh->normals().clear();
		mesh->uvs().clear();
		mesh->tangents().clear();
		mesh->bitangents().clear();
		mesh->lightmaps().clear();
		mesh->colors().clear();
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