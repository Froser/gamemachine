#include "stdafx.h"
#include "gmglobjectpainter.h"
#include "shader_constants.h"
#include "gmglfunc.h"
#include "gmgllight.h"
#include "gmglshadowmapping.h"
#include "utilities/assert.h"

GMGLObjectPainter::GMGLObjectPainter(GMGLShaders& shaders, GMGLShadowMapping& shadowMapping, Object* obj)
	: ObjectPainter(obj)
	, m_shaders(shaders)
	, m_shadowMapping(shadowMapping)
{
}

void GMGLObjectPainter::init()
{
	Object* obj = getObject();

	GLuint vao[1];
	glGenVertexArrays(1, &vao[0]);
	glBindVertexArray(vao[0]);
	getObject()->setArrayId(vao[0]);

	GLuint vaoSize = sizeof(Object::DataType) * obj->vertices().size();
	GLuint normalSize = sizeof(Object::DataType) * obj->normals().size();
	GLuint uvSize = sizeof(Object::DataType) * obj->uvs().size();

	GLuint vbo[1];
	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vaoSize + normalSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vaoSize, obj->vertices().data());
	glBufferSubData(GL_ARRAY_BUFFER, vaoSize, normalSize, obj->normals().data());
	glBufferSubData(GL_ARRAY_BUFFER, normalSize, uvSize, obj->uvs().data());
	obj->setBufferId(vbo[0]);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, (void*)vaoSize);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)normalSize);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	obj->disposeMemory();
}

void GMGLObjectPainter::draw()
{
	Object* obj = getObject();

	glBindVertexArray(obj->getArrayId());
	GLint params[2];
	glGetIntegerv(GL_POLYGON_MODE, params);
	
	for (auto iter = obj->getComponents().cbegin(); iter != obj->getComponents().cend(); iter++)
	{
		Component* component = (*iter);
		ITexture* texture = component->getMaterial().texture;

		if (!m_shadowMapping.hasBegun())
		{
			GMGLLight(m_shaders).setAmbientCoefficient(component->getMaterial().Ka);
			GMGLLight(m_shaders).setDiffuseCoefficient(component->getMaterial().Kd);
			GMGLLight(m_shaders).setSpecularCoefficient(component->getMaterial().Ks);
			GMGLLight(m_shaders).setShininess(component->getMaterial().shininess);

			if (texture)
				component->getMaterial().texture->beginTexture();
		}

		glMultiDrawArrays(params[1] == GL_FILL ? GL_TRIANGLE_FAN : GL_LINE_LOOP,
			component->getFirstPtr(), component->getCountPtr(), component->getPolygonCount());

		if (!m_shadowMapping.hasBegun() && texture)
			texture->endTexture();
	}
	glBindVertexArray(0);
}

void GMGLObjectPainter::dispose()
{
	Object* obj = getObject();

	GLuint vao[1] = { obj->getArrayId() },
		vbo[1] = { obj->getBufferId() };

	glDeleteVertexArrays(1, vao);
	glDeleteBuffers(1, vbo);
}
