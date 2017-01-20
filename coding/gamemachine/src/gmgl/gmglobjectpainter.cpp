#include "stdafx.h"
#include "gmglobjectpainter.h"
#include "shader_constants.h"
#include "gmglfunc.h"
#include "gmgllight.h"
#include "gmglshadowmapping.h"
#include "utilities/assert.h"
#include "gmgltexture.h"
#include "gmengine/elements/gameworld.h"
#include "gmglgraphic_engine.h"

GLenum getMode(ChildObject* obj)
{
	switch (obj->getArrangementMode())
	{
	case ChildObject::Triangle_Fan:
		return GL_TRIANGLE_FAN;
	case ChildObject::Triangle_Strip:
		return GL_TRIANGLE_STRIP;
	default:
		ASSERT(false);
		return GL_TRIANGLE_FAN;
	}
}

GMGLObjectPainter::GMGLObjectPainter(IGraphicEngine* engine, GMGLShadowMapping& shadowMapping, Object* objs)
	: ObjectPainter(objs)
	, m_engine(static_cast<GMGLGraphicEngine*>(engine))
	, m_shadowMapping(shadowMapping)
	, m_inited(false)
	, m_world(nullptr)
{
}

void GMGLObjectPainter::transfer()
{
	if (m_inited)
		return;

	Object* obj = getObject();
	BEGIN_FOREACH_OBJ(obj, childObj)
	{
		childObj->calculateTangentSpace();

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		childObj->setArrayId(vao);

		GLuint vaoSize = sizeof(Object::DataType) * childObj->vertices().size();
		GLuint normalSize = sizeof(Object::DataType) * childObj->normals().size();
		GLuint uvSize = sizeof(Object::DataType) * childObj->uvs().size();
		GLuint tangentSize = sizeof(Object::DataType) * childObj->tangents().size();

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize + tangentSize, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vaoSize, childObj->vertices().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize, normalSize, childObj->normals().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize, uvSize, childObj->uvs().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize, tangentSize, childObj->tangents().data());
		childObj->setBufferId(vbo);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, (void*)vaoSize);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vaoSize + normalSize));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		childObj->disposeMemory();
	}
	END_FOREACH_OBJ

	m_inited = true;
}

void GMGLObjectPainter::draw()
{
	Object* obj = getObject();

	BEGIN_FOREACH_OBJ(obj, childObj)
	{
		glBindVertexArray(childObj->getArrayId());
		GLint params[2];
		glGetIntegerv(GL_POLYGON_MODE, params);
		resetTextures(childObj->getType());

		for (auto iter = childObj->getComponents().cbegin(); iter != childObj->getComponents().cend(); iter++)
		{
			Component* component = (*iter);
			TextureInfo* textureInfos = component->getMaterial().textures;

			if (!m_shadowMapping.hasBegun())
			{
				setLights(component->getMaterial(), childObj->getType());
				beginTextures(textureInfos, childObj->getType());
			}

			GLenum mode = getMode(childObj);
			glMultiDrawArrays(params[1] == GL_FILL ? mode : GL_LINE_LOOP,
				component->getOffsetPtr(), component->getEdgeCountPtr(), component->getPolygonCount());

			if (!m_shadowMapping.hasBegun())
				endTextures(textureInfos);
		}
		glBindVertexArray(0);
	}
	END_FOREACH_OBJ
}

void GMGLObjectPainter::dispose()
{
	Object* obj = getObject();

	BEGIN_FOREACH_OBJ(obj, childObj)
	{
		GLuint vao[1] = { childObj->getArrayId() },
			vbo[1] = { childObj->getBufferId() };

		glDeleteVertexArrays(1, vao);
		glDeleteBuffers(1, vbo);
	}
	END_FOREACH_OBJ

	m_inited = false;
}

void GMGLObjectPainter::clone(Object* obj, OUT ObjectPainter** painter)
{
	ASSERT(painter);
	*painter = new GMGLObjectPainter(m_engine, m_shadowMapping, obj);
}

void GMGLObjectPainter::setLights(Material& material, ChildObject::ObjectType type)
{
	if (m_world)
	{
		std::vector<GameLight*>& lights = m_world->getLights();

		for (auto iter = lights.begin(); iter != lights.end(); iter++)
		{
			GMGLLight* light = static_cast<GMGLLight*>(*iter);
			if (light->isAvailable())
			{
				light->setShaders(m_engine->getShaders(type));
				light->activateLight(material);
			}
		}
	}
}

void GMGLObjectPainter::beginTextures(TextureInfo* startTexture, ChildObject::ObjectType type)
{
	for (GMuint i = 0; i < MaxTextureCount; i++)
	{
		TextureInfo& info = startTexture[i];
		ITexture* t = info.texture;
		if (t)
		{
			t->beginTexture(info.type);
			GMGL::uniformTextureIndex(*m_engine->getShaders(type), info.type, getTextureUniformName(info.type));
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

void GMGLObjectPainter::resetTextures(ChildObject::ObjectType type)
{
	for (TextureType t = TextureTypeAmbient; t < TextureTypeResetEnd; t = (TextureType) ((GMuint)(t) + 1))
	{
		GMGL::disableTexture(*m_engine->getShaders(type), getTextureUniformName(t));
	}
}

void GMGLObjectPainter::setWorld(GameWorld* world)
{
	m_world = world;
}
