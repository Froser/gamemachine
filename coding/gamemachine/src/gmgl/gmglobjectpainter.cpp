#include "stdafx.h"
#include "gmglobjectpainter.h"
#include "shader_constants.h"
#include "gmglfunc.h"
#include "gmgllight.h"
#include "gmglshadowmapping.h"
#include "utilities/assert.h"
#include "gmgltexture.h"

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
	glBufferData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vaoSize, obj->vertices().data());
	glBufferSubData(GL_ARRAY_BUFFER, vaoSize, normalSize, obj->normals().data());
	glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize, uvSize, obj->uvs().data());
	obj->setBufferId(vbo[0]);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, (void*)vaoSize);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vaoSize + normalSize));
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
		TextureInfo* textureInfos = component->getMaterial().textures;

		if (!m_shadowMapping.hasBegun())
		{
			setLights(component->getMaterial());
			beginTextures(textureInfos);
		}

		glMultiDrawArrays(params[1] == GL_FILL ? GL_TRIANGLE_FAN : GL_LINE_LOOP,
			component->getFirstPtr(), component->getCountPtr(), component->getPolygonCount());

		if (!m_shadowMapping.hasBegun())
			endTextures(textureInfos);
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

void GMGLObjectPainter::setLights(Material& material)
{
	GMGLLight light(m_shaders);
	light.setAmbientCoefficient(material.Ka);
	light.setDiffuseCoefficient(material.Kd);
	light.setSpecularCoefficient(material.Ks);
	light.setShininess(material.shininess);
}

void GMGLObjectPainter::beginTextures(TextureInfo* startTexture)
{
	bool hasTexture = false;
	for (GMuint i = 0; i < MaxTextureCount; i++)
	{
		TextureInfo& info = startTexture[i];
		ITexture* t = info.texture;
		if (t)
		{
			t->beginTexture(info.type);
			GMGL::uniformTextureIndex(m_shaders, info.type, getTextureUniformName(info.type));
			hasTexture = true;
		}
	}

	if (hasTexture)
		textureCalculateSwitch(1);
	else
		textureCalculateSwitch(0);
}

void GMGLObjectPainter::endTextures(TextureInfo* startTexture)
{
	for (GMuint i = 0; i < MaxTextureCount; i++)
	{
		ITexture* t = startTexture[i].texture;
		if (t)
		{
			t->endTexture();
		}
	}
}

void GMGLObjectPainter::textureCalculateSwitch(bool on)
{
	glUniform1i(glGetUniformLocation(m_shaders.getProgram(), GMSHADER_ENABLE_TEXTURE), on ? 1 : 0);
}