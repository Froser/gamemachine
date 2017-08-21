#include "stdafx.h"
#include "gmgl_renders_lightpass.h"
#include "foundation/gamemachine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/gmglshaderprogram.h"

GMGLRenders_LightPass::GMGLRenders_LightPass()
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(GameMachine::instance().getGraphicEngine());
}

void GMGLRenders_LightPass::activateLight(const GMLight& light, GMint lightIndex)
{
	D(d);
	auto shaderProgram = d->engine->getLightPassShader();
	shaderProgram->useProgram();

	switch (light.getType())
	{
	case GMLightType::AMBIENT:
		{
			const char* uniform = getLightUniformName(GMLightType::AMBIENT, lightIndex);
			char u_color[GMGL_MAX_UNIFORM_NAME_LEN];
			combineUniform(u_color, uniform, GMSHADER_LIGHTS_LIGHTCOLOR);
			shaderProgram->setVec3(u_color, light.getLightColor());
		}
		break;
	case GMLightType::SPECULAR:
		{
			const char* uniform = getLightUniformName(GMLightType::SPECULAR, lightIndex);
			char u_color[GMGL_MAX_UNIFORM_NAME_LEN], u_position[GMGL_MAX_UNIFORM_NAME_LEN];
			combineUniform(u_color, uniform, GMSHADER_LIGHTS_LIGHTCOLOR);
			combineUniform(u_position, uniform, GMSHADER_LIGHTS_LIGHTPOSITION);
			shaderProgram->setVec3(u_color, light.getLightColor());
			shaderProgram->setVec3(u_position, light.getLightPosition());
			GLenum errCode;
			ASSERT((errCode = glGetError()) == GL_NO_ERROR);
		}
		break;
	default:
		break;
	}
}

void GMGLRenders_LightPass::updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt)
{
	D(d);
	static auto identity = linear_math::Matrix4x4::identity();
	auto shaderProgram = d->engine->getLightPassShader();
	shaderProgram->useProgram();
	shaderProgram->setMatrix4(GMSHADER_MODEL_MATRIX, identity.data());
	shaderProgram->setMatrix4(GMSHADER_VIEW_MATRIX, view.data());
}