#include "stdafx.h"
#include "gmglobjectpainter.h"
#include "shader_constants.h"
#include "gmglfunc.h"
#include "gmgllight.h"
#include "gmglshadowmapping.h"
#include "utilities/assert.h"
#include "gmgltexture.h"
#include "gmengine/elements/gameworld.h"

GMGLObjectPainter::GMGLObjectPainter(GMGLShaders& shaders, GMGLShadowMapping& shadowMapping, Object* obj)
	: ObjectPainter(obj)
	, m_shaders(shaders)
	, m_shadowMapping(shadowMapping)
	, m_inited(false)
	, m_world(nullptr)
{
}

void GMGLObjectPainter::init()
{
	if (m_inited)
		return;

	Object* obj = getObject();

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	getObject()->setArrayId(vao);

	GLuint vaoSize = sizeof(Object::DataType) * obj->vertices().size();
	GLuint normalSize = sizeof(Object::DataType) * obj->normals().size();
	GLuint uvSize = sizeof(Object::DataType) * obj->uvs().size();

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vaoSize, obj->vertices().data());
	glBufferSubData(GL_ARRAY_BUFFER, vaoSize, normalSize, obj->normals().data());
	glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize, uvSize, obj->uvs().data());
	obj->setBufferId(vbo);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, (void*)vaoSize);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vaoSize + normalSize));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	obj->disposeMemory();

	m_inited = true;
}

void GMGLObjectPainter::draw()
{
	Object* obj = getObject();

	glBindVertexArray(obj->getArrayId());
	GLint params[2];
	glGetIntegerv(GL_POLYGON_MODE, params);
	resetTextures();
	
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

	m_inited = false;
}

void GMGLObjectPainter::setLights(Material& material)
{
	if (m_world)
	{
		std::vector<GameLight*>& lights = m_world->getLights();

		for (auto iter = lights.begin(); iter != lights.end(); iter++)
		{
			GameLight* light = (*iter);
			if (light->isAvailable())
				light->activateLight(material);
		}
	}
}

void GMGLObjectPainter::beginTextures(TextureInfo* startTexture)
{
	for (GMuint i = 0; i < MaxTextureCount; i++)
	{
		TextureInfo& info = startTexture[i];
		ITexture* t = info.texture;
		if (t)
		{
			t->beginTexture(info.type);
			GMGL::uniformTextureIndex(m_shaders, info.type, getTextureUniformName(info.type));
		}
	}
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

void GMGLObjectPainter::resetTextures()
{
	for (TextureType t = TextureTypeAmbient; t < TextureTypeResetEnd; t = (TextureType) ((GMuint)(t) + 1))
	{
		GMGL::disableTexture(m_shaders, getTextureUniformName(t));
	}
}

void GMGLObjectPainter::setWorld(GameWorld* world)
{
	m_world = world;
}
