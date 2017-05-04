#include "stdafx.h"
#include "gmglobjectpainter.h"
#include "shader_constants.h"
#include "gmglfunc.h"
#include "gmglshadowmapping.h"
#include "utilities/assert.h"
#include "gmgltexture.h"
#include "gmengine/elements/gameworld.h"
#include "gmglgraphic_engine.h"

static GLenum getMode(ChildObject* obj)
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

		childObj->vertices().clear();
		childObj->normals().clear();
		childObj->uvs().clear();
		childObj->tangents().clear();
		childObj->bitangents().clear();
		childObj->lightmaps().clear();
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
			Shader& shader = component->getShader();
			if (shader.nodraw)
				continue;

			if (!m_shadowMapping.hasBegun())
			{
				for (GMint i = LT_BEGIN; i < LT_END; i++)
				{
					activateLight((LightType)i, shader.lights[i], childObj->getType());
				}
				beginShader(&shader, childObj->getType());
			}

			drawDebug(childObj->getType());
			GLenum mode = DBG_INT(POLYGON_LINE_MODE) ? GL_LINE_LOOP : getMode(childObj);
			glMultiDrawArrays(mode, component->getOffsetPtr(), component->getPrimitiveVerticesCountPtr(), component->getPrimitiveCount());

			if (!m_shadowMapping.hasBegun())
				endShader(&shader, childObj->getType());
		}
		glBindVertexArray(0);
	}
	END_FOREACH_OBJ
}

void GMGLObjectPainter::activateLight(LightType t, LightInfo& light, ChildObject::ObjectType objectType)
{
	GMGLShaders* gmglShaders = m_engine->getShaders(objectType);
	switch (t)
	{
	case gm::LT_AMBIENT:
		{
			GMGL::uniformVec3(*gmglShaders, light.on && !light.useGlobalLightColor ? &light.lightColor[0] : &(m_world->getDefaultAmbientLight().lightColor)[0], GMSHADER_LIGHT_AMBIENT);
			GMGL::uniformVec3(*gmglShaders, light.on ? &light.args[LA_KA] : &m_world->getDefaultAmbientLight().args[LA_KA], GMSHADER_LIGHT_KA);
		}
		break;
	case gm::LT_SPECULAR:
		{
			GMfloat zero[3] = { 0 };
			GMGL::uniformVec3(*gmglShaders, light.on ? (!light.useGlobalLightColor ? &light.lightColor[0] : &(m_world->getDefaultAmbientLight().lightColor)[0]) : zero, GMSHADER_LIGHT_POWER);
			GMGL::uniformVec3(*gmglShaders, light.on ? &light.args[LA_KD] : zero, GMSHADER_LIGHT_KD);
			GMGL::uniformVec3(*gmglShaders, light.on ? &light.args[LA_KS] : zero, GMSHADER_LIGHT_KS);
			GMGL::uniformFloat(*gmglShaders, light.on ? light.args[LA_SHINESS] : 0.f, GMSHADER_LIGHT_SHININESS);
		}
		break;
	default:
		ASSERT(false);
		break;
	}
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

void GMGLObjectPainter::activeTexture(Shader* shader, TextureIndex i, ChildObject::ObjectType type)
{
	std::string uniform = getTextureUniformName(i);
	GLint loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), uniform.c_str());
	glUniform1i(loc, i + 1);
	loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), std::string(uniform).append("_switch").c_str());
	glUniform1i(loc, 1);

	activeTextureTransform(shader, i, type);
	glActiveTexture(i + GL_TEXTURE1);
}

void GMGLObjectPainter::activeTextureTransform(Shader* shader, TextureIndex i, ChildObject::ObjectType type)
{
	std::string uniform = getTextureUniformName(i);

	const std::string SCROLL_S = std::string(uniform).append("_scroll_s");
	const std::string SCROLL_T = std::string(uniform).append("_scroll_t");
	const std::string SCALE_S = std::string(uniform).append("_scale_s");
	const std::string SCALE_T = std::string(uniform).append("_scale_t");

	glUniform1f(glGetUniformLocation(m_engine->getShaders(type)->getProgram(), SCROLL_S.c_str()), 0.f);
	glUniform1f(glGetUniformLocation(m_engine->getShaders(type)->getProgram(), SCROLL_T.c_str()), 0.f);
	glUniform1f(glGetUniformLocation(m_engine->getShaders(type)->getProgram(), SCALE_S.c_str()), 1.f);
	glUniform1f(glGetUniformLocation(m_engine->getShaders(type)->getProgram(), SCALE_T.c_str()), 1.f);

	GMuint n = 0;
	while (n < MAX_TEX_MOD && shader->texture.textures[i].texMod[n].type != GMS_NO_TEXTURE_MOD)
	{
		GMS_TextureMod* tc = &shader->texture.textures[i].texMod[n];
		switch (tc->type)
		{
		case GMS_SCROLL:
			{
				GMfloat s = m_world->getElapsed() * tc->p1, t = m_world->getElapsed() * tc->p2;
				glUniform1f(glGetUniformLocation(m_engine->getShaders(type)->getProgram(), SCROLL_T.c_str()), t);
				glUniform1f(glGetUniformLocation(m_engine->getShaders(type)->getProgram(), SCROLL_T.c_str()), s);
			}
			break;
		case GMS_SCALE:
			{
				GMfloat s = tc->p1, t = tc->p2;
				glUniform1f(glGetUniformLocation(m_engine->getShaders(type)->getProgram(), SCALE_T.c_str()), t);
				glUniform1f(glGetUniformLocation(m_engine->getShaders(type)->getProgram(), SCALE_T.c_str()), s);
				break;
			}
		default:
			break;
		}
		n++;
	}

}

void GMGLObjectPainter::deactiveTexture(TextureIndex i, ChildObject::ObjectType type)
{
	std::string uniform = getTextureUniformName(i);
	GMint loc = glGetUniformLocation(m_engine->getShaders(type)->getProgram(), std::string(uniform).append("_switch").c_str());
	glUniform1i(loc, 0);
	glActiveTexture(i + GL_TEXTURE1);
}

ITexture* GMGLObjectPainter::getTexture(TextureFrames& frames)
{
	if (frames.frameCount == 0)
		return nullptr;

	if (frames.frameCount == 1)
		return frames.frames[0];

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint elapsed = m_world->getElapsed() * 1000;

	return frames.frames[(elapsed / frames.animationMs) % frames.frameCount];
}

void GMGLObjectPainter::activateShader(Shader* shader)
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
			case GMS_DST_COLOR:
				factors[i] = GL_DST_COLOR;
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

	if (shader->noDepthTest)
		glDisable(GL_DEPTH_TEST); //glDepthMask(GL_FALSE);
	else
		glEnable(GL_DEPTH_TEST); // glDepthMask(GL_TRUE);
}

void GMGLObjectPainter::deactivateShader(Shader* shader)
{
	if (shader->blend)
	{
		glDepthMask(GL_TRUE);
	}
}

void GMGLObjectPainter::beginShader(Shader* shader, ChildObject::ObjectType type)
{
	activateShader(shader);
	for (GMuint i = 0; i < TEXTURE_INDEX_MAX; i++)
	{
		// 按照贴图类型选择纹理动画序列
		TextureFrames& textures = shader->texture.textures[i];

		// 获取序列中的这一帧
		ITexture* texture = getTexture(textures);
		if (texture)
		{
			// 激活动画序列
			activeTexture(shader, (TextureIndex)i, type);
			texture->drawTexture(&textures);
		}
	}
}

void GMGLObjectPainter::endShader(Shader* shader, ChildObject::ObjectType type)
{
	deactivateShader(shader);
	for (GMuint i = 0; i < TEXTURE_INDEX_MAX; i++)
	{
		deactiveTexture((TextureIndex)i, type);
	}
}

void GMGLObjectPainter::setWorld(GameWorld* world)
{
	m_world = world;
}

// 是否需要做一些调试方面的绘制
void GMGLObjectPainter::drawDebug(ChildObject::ObjectType type)
{
	GMGL::uniformInt(*m_engine->getShaders(type), DBG_INT(DRAW_NORMAL), GMSHADER_DEBUG_DRAW_NORMAL);
}