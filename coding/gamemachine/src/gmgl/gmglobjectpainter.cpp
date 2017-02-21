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
	case ChildObject::Triangles:
		return GL_TRIANGLES;
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
		GLuint bitangentSize = sizeof(Object::DataType) * childObj->bitangents().size();
		GLuint lightmapSize = sizeof(Object::DataType) * childObj->lightmaps().size();

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0															, vaoSize, childObj->vertices().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize													, normalSize, childObj->normals().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize										, uvSize, childObj->uvs().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize								, tangentSize, childObj->tangents().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize + tangentSize				, bitangentSize, childObj->bitangents().data());
		glBufferSubData(GL_ARRAY_BUFFER, vaoSize + normalSize + uvSize + tangentSize + lightmapSize	, lightmapSize, childObj->lightmaps().data());
		childObj->setBufferId(vbo);

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
		if (!childObj->getVisibility())
			continue;

		glBindVertexArray(childObj->getArrayId());
		GLint params[2];
		glGetIntegerv(GL_POLYGON_MODE, params);
		GLenum mode = params[1] == GL_FILL ? mode : GL_LINE_LOOP;
//		resetTextures(childObj->getType());

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
			glMultiDrawArrays(mode, component->getOffsetPtr(), component->getPrimitiveVerticesCountPtr(), component->getPrimitiveCount());

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
		// 开始绘制一套纹理

		TextureInfo& info = startTexture[i];
		ITexture* t = info.texture;
		if (t)
		{
			GLint loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), "GM_ambient_texture");
			glUniform1i(loc, 1);
			loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), "GM_ambient_texture_switch");
			glUniform1i(loc, 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, ((GMGLTexture*)t)->textureId());

			//loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), "GM_lightmap_texture_switch");
			//glUniform1i(loc, 1);
			//GMGL::uniformTextureIndex(*m_engine->getShaders(type), info.type, getTextureUniformName(info.type));
			//t->beginTexture(info.type);
		}

		t = info.normalMapping;
		if (t)
		{
			GMGL::uniformTextureIndex(*m_engine->getShaders(type), TextureTypeNormalMapping, getTextureUniformName(TextureTypeNormalMapping));
			t->beginTexture(TextureTypeNormalMapping);
		}

		t = info.lightmap;
		if (t)
		{
			GLint loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), "GM_lightmap_texture");
			glUniform1i(loc, 5);
			loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), "GM_lightmap_texture_switch");
			glUniform1i(loc, 1);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, ((GMGLTexture*)t)->textureId());
		}
	}
}

void GMGLObjectPainter::endTextures(TextureInfo* startTexture)
{
	for (GMuint i = 0; i < MaxTextureCount; i++)
	{
		ITexture* t = startTexture[i].texture;
		if (t)
			t->endTexture();
		//
		//t = startTexture[i].normalMapping;
		//if (t)
		//	t->endTexture();
		//
		//t = startTexture[i].lightmap;
		//if (t)
		//	t->endTexture();
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
