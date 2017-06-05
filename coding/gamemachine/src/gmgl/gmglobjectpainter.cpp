#include "stdafx.h"
#include "gmglobjectpainter.h"
#include "shader_constants.h"
#include "gmglfunc.h"
#include "gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include "gmglgraphic_engine.h"
#include "renders/gmgl_render.h"
#include "foundation/gamemachine.h"

static GLenum getMode(Mesh* obj)
{
	switch (obj->getArrangementMode())
	{
	case Mesh::Triangle_Fan:
		return GL_TRIANGLE_FAN;
	case Mesh::Triangle_Strip:
		return GL_TRIANGLE_STRIP;
	case Mesh::Triangles:
		return GL_TRIANGLES;
	default:
		ASSERT(false);
		return GL_TRIANGLE_FAN;
	}
}

GMGLObjectPainter::GMGLObjectPainter(IGraphicEngine* engine, Object* objs)
	: ObjectPainter(objs)
	, m_engine(static_cast<GMGLGraphicEngine*>(engine))
	, m_inited(false)
{
}

void GMGLObjectPainter::transfer()
{
	if (m_inited)
		return;

	Object* obj = getObject();
	BEGIN_FOREACH_OBJ(obj, mesh)
	{
		mesh->calculateTangentSpace();

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		mesh->setArrayId(vao);

		GLuint vaoSize = sizeof(Object::DataType) * mesh->vertices().size();
		GLuint normalSize = sizeof(Object::DataType) * mesh->normals().size();
		GLuint uvSize = sizeof(Object::DataType) * mesh->uvs().size();
		GLuint tangentSize = sizeof(Object::DataType) * mesh->tangents().size();
		GLuint bitangentSize = sizeof(Object::DataType) * mesh->bitangents().size();
		GLuint lightmapSize = sizeof(Object::DataType) * mesh->lightmaps().size();

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0															, vaoSize, mesh->vertices().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize													, normalSize, mesh->normals().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize										, uvSize, mesh->uvs().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize								, tangentSize, mesh->tangents().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize + tangentSize				, bitangentSize, mesh->bitangents().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize + tangentSize + lightmapSize	, lightmapSize, mesh->lightmaps().data());
		mesh->setBufferId(vbo);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, (void*)vaoSize);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vaoSize + normalSize));
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)(vaoSize + normalSize + uvSize));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, (void*)(vaoSize + normalSize + uvSize + tangentSize));
		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vaoSize + normalSize + uvSize + tangentSize + lightmapSize));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);

		glBindVertexArray(0);

		mesh->vertices().clear();
		mesh->normals().clear();
		mesh->uvs().clear();
		mesh->tangents().clear();
		mesh->bitangents().clear();
		mesh->lightmaps().clear();
	}
	END_FOREACH_OBJ

	m_inited = true;
}

void GMGLObjectPainter::draw(GMfloat* modelTransform)
{
	Object* obj = getObject();

	BEGIN_FOREACH_OBJ(obj, mesh)
	{
		IRender* render = m_engine->getRender(mesh->getType());
		render->begin(m_engine, mesh, modelTransform);

		glBindVertexArray(mesh->getArrayId());
		for (auto iter = mesh->getComponents().begin(); iter != mesh->getComponents().end(); iter++)
		{
			Component* component = (*iter);
			Shader& shader = component->getShader();
			if (shader.nodraw)
				continue;

			render->beginShader(shader);
			GLenum mode = GMGetBuiltIn(POLYGON_LINE_MODE) ? GL_LINE_LOOP : getMode(mesh);
			glMultiDrawArrays(mode, component->getOffsetPtr(), component->getPrimitiveVerticesCountPtr(), component->getPrimitiveCount());
			render->endShader();
		}
		glBindVertexArray(0);

		render->end();
	}
	END_FOREACH_OBJ
}

void GMGLObjectPainter::dispose()
{
	Object* obj = getObject();

	BEGIN_FOREACH_OBJ(obj, mesh)
	{
		GLuint vao[1] = { mesh->getArrayId() },
			vbo[1] = { mesh->getBufferId() };

		glDeleteVertexArrays(1, vao);
		glDeleteBuffers(1, vbo);
	}
	END_FOREACH_OBJ

	m_inited = false;
}

void GMGLObjectPainter::clone(Object* obj, OUT ObjectPainter** painter)
{
	ASSERT(painter);
	*painter = new GMGLObjectPainter(m_engine, obj);
}