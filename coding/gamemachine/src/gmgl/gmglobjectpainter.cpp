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
		glBindVertexArray(childObj->getArrayId());

		for (auto iter = childObj->getComponents().cbegin(); iter != childObj->getComponents().cend(); iter++)
		{
			Component* component = (*iter);
			Shader& shader = component->getMaterial().shader;
			if (shader.nodraw)
				continue;

			if (!m_shadowMapping.hasBegun())
			{
				setLights(component->getMaterial(), childObj->getType());
				beginShader(&shader, childObj->getType());
			}

			GLenum mode = DBG_INT(POLYGON_LINE_MODE) ? GL_LINE_LOOP : getMode(childObj);
			glMultiDrawArrays(mode, component->getOffsetPtr(), component->getPrimitiveVerticesCountPtr(), component->getPrimitiveCount());

			if (!m_shadowMapping.hasBegun())
				endShader(&shader, childObj->getType());
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

void GMGLObjectPainter::activeTexture(TextureIndex i, ChildObject::ObjectType type)
{
	std::string uniform = getTextureUniformName(i);
	GLint loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), uniform.c_str());
	glUniform1i(loc, i + 1);
	loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), uniform.append("_switch").c_str());
	glUniform1i(loc, 1);
	glActiveTexture(i + GL_TEXTURE1);
}

void GMGLObjectPainter::deactiveTexture(TextureIndex i, ChildObject::ObjectType type)
{
	std::string uniform = getTextureUniformName(i);
	GMint loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), uniform.append("_switch").c_str());
	glUniform1i(loc, 0);
	glActiveTexture(i + GL_TEXTURE1);
}

ITexture* GMGLObjectPainter::getTexture(TextureFrames& frames)
{
	if (frames.frameCount == 0)
		return nullptr;

	if (frames.frameCount == 1)
		return frames.textures[0];

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint elapsed = m_world->getElapsed() * 1000;

	return frames.textures[(elapsed / frames.animationMs) % frames.frameCount];
}

void GMGLObjectPainter::activeShader(Shader* shader)
{
	if (shader->cull == GMS_NONE)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);
	}

	if (shader->blend)
	{
		glEnable(GL_BLEND);
		GLenum factors[2];
		for (GMuint i = 0; i < 2; i++)
		{
			switch (shader->blendFactors[i])
			{
			case GMS_ZERO:
				factors[i] = GL_ZERO;
				break;
			case GMS_ONE:
				factors[i] = GL_ONE;
				break;
			default:
				ASSERT(false);
				break;
			}
		}
		glBlendFunc(factors[0], factors[1]);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	if (shader->blend)
		glDepthMask(GL_FALSE);

	if (shader->nodepthmask)
		glDepthMask(GL_FALSE);
	else
		glDepthMask(GL_TRUE);
}

void GMGLObjectPainter::deactiveShader(Shader* shader)
{
	if (shader->blend)
	{
		glDepthMask(GL_TRUE);
	}
}

void GMGLObjectPainter::beginShader(Shader* shader, ChildObject::ObjectType type)
{
	activeShader(shader);
	for (GMuint i = 0; i < TEXTURE_INDEX_MAX; i++)
	{
		// 按照贴图类型选择纹理动画序列
		TextureFrames& frames = shader->texture.textureFrames[i];

		// 获取序列中的这一帧
		ITexture* texture = getTexture(frames);
		if (texture)
		{
			// 激活动画序列
			activeTexture((TextureIndex)i, type);
			texture->drawTexture(&frames);
		}
	}
}

void GMGLObjectPainter::endShader(Shader* shader, ChildObject::ObjectType type)
{
	deactiveShader(shader);
	for (GMuint i = 0; i < TEXTURE_INDEX_MAX; i++)
	{
		deactiveTexture((TextureIndex)i, type);
	}
}

void GMGLObjectPainter::setWorld(GameWorld* world)
{
	m_world = world;
}
