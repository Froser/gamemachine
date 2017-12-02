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

void GMGLRenders_LightPass::activateLights(const GMLight* lights, GMint count)
{
	D(d);
	auto shaderProgram = d->engine->getShaderProgram();
	shaderProgram->useProgram();

	GMint lightId[(GMuint)GMLightType::COUNT] = { 0 };
	for (GMint i = 0; i < count; i++)
	{
		const GMLight& light = lights[i];
		GMint id = lightId[(GMuint)light.getType()]++;

		switch (light.getType())
		{
		case GMLightType::AMBIENT:
			{
				const char* uniform = getLightUniformName(GMLightType::AMBIENT, id);
				char u_color[GMGL_MAX_UNIFORM_NAME_LEN];
				combineUniform(u_color, uniform, GMSHADER_LIGHTS_LIGHTCOLOR);
				shaderProgram->setVec3(u_color, light.getLightColor());
			}
			break;
		case GMLightType::SPECULAR:
			{
				const char* uniform = getLightUniformName(GMLightType::SPECULAR, id);
				char u_color[GMGL_MAX_UNIFORM_NAME_LEN], u_position[GMGL_MAX_UNIFORM_NAME_LEN];
				combineUniform(u_color, uniform, GMSHADER_LIGHTS_LIGHTCOLOR);
				combineUniform(u_position, uniform, GMSHADER_LIGHTS_LIGHTPOSITION);
				shaderProgram->setVec3(u_color, light.getLightColor());
				shaderProgram->setVec3(u_position, light.getLightPosition());
				GM_CHECK_GL_ERROR();
			}
			break;
		default:
			break;
		}
	}

	shaderProgram->setInt(GMSHADER_AMBIENTS_COUNT, lightId[(GMint)GMLightType::AMBIENT]);
	shaderProgram->setInt(GMSHADER_SPECULARS_COUNT, lightId[(GMint)GMLightType::SPECULAR]);
}