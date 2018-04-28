#include "stdafx.h"
#include "gmgllight.h"
#include "gmglrenderers.h"
#include "shader_constants.h"

template <GMuint i>
struct String {};

#define MAKE_STRING_MAP(i) template <> struct String<i> { static constexpr const char* Value = #i; };
MAKE_STRING_MAP(0)
MAKE_STRING_MAP(1)
MAKE_STRING_MAP(2)
MAKE_STRING_MAP(3)
MAKE_STRING_MAP(4)
MAKE_STRING_MAP(5)
MAKE_STRING_MAP(6)
MAKE_STRING_MAP(7)

#define BEGIN_QUERY_STRING(i) switch(i) {
#define ADD_QUERY_STRING(number) case number: return String<number>::Value;
#define END_QUERY_STRING() default: GM_ASSERT(false); return ""; }

namespace
{
	const char* number(GMint index)
	{
		BEGIN_QUERY_STRING(index);
			ADD_QUERY_STRING(0);
			ADD_QUERY_STRING(1);
			ADD_QUERY_STRING(2);
			ADD_QUERY_STRING(3);
			ADD_QUERY_STRING(4);
			ADD_QUERY_STRING(5);
			ADD_QUERY_STRING(6);
			ADD_QUERY_STRING(7);
		END_QUERY_STRING()
	}
}

void GMGLLight::setLightPosition(GMfloat position[3])
{
	D(d);
	d->position[0] = position[0];
	d->position[1] = position[1];
	d->position[2] = position[2];
	d->position[3] = 1.0f;
}

void GMGLLight::setLightColor(GMfloat color[3])
{
	D(d);
	d->color[0] = color[0];
	d->color[1] = color[1];
	d->color[2] = color[2];
	d->color[3] = 1.0f;
}

const GMfloat* GMGLLight::getLightPosition() const
{
	D(d);
	return d->position;
}

const GMfloat* GMGLLight::getLightColor() const
{
	D(d);
	return d->color;
}

void GMGLLight::activateLight(GMuint index, IRenderer* renderer)
{
	static char light_Position[GMGL_MAX_UNIFORM_NAME_LEN];
	static char light_Color[GMGL_MAX_UNIFORM_NAME_LEN];
	static char light_Type[GMGL_MAX_UNIFORM_NAME_LEN];

	GMGLRenderer* glRenderer = gm_cast<GMGLRenderer*>(renderer);
	IShaderProgram* shaderProgram = glRenderer->getShaderProgram();
	const GMShaderVariablesDesc& desc = shaderProgram->getDesc();
	const char* strIndex = number(index);

	combineUniform(light_Position, "GM_lights[", strIndex, "].lightColor");
	shaderProgram->setVec3(light_Position, getLightColor());

	combineUniform(light_Color, "GM_lights[", strIndex, "].lightPosition");
	shaderProgram->setVec3(light_Color, getLightPosition());

	combineUniform(light_Type, "GM_lights[", strIndex, "].lightType");
	shaderProgram->setInt(light_Type, getType());
}