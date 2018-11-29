#include "stdafx.h"
#include "gmgllight.h"
#include "gmgltechniques.h"
#include "shader_constants.h"

template <GMuint32 i>
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
	const char* number(GMint32 index)
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

void GMGLLight::activateLight(GMuint32 index, ITechnique* technique)
{
	D(d);
	static char light_Position[GMGL_MAX_UNIFORM_NAME_LEN] = { 0 };
	static char light_Color[GMGL_MAX_UNIFORM_NAME_LEN] = { 0 };
	static char light_Type[GMGL_MAX_UNIFORM_NAME_LEN] = { 0 };
	static char light_Attenuation_Constant[GMGL_MAX_UNIFORM_NAME_LEN] = { 0 };
	static char light_Attenuation_Linear[GMGL_MAX_UNIFORM_NAME_LEN] = { 0 };
	static char light_Attenuation_Exp[GMGL_MAX_UNIFORM_NAME_LEN] = { 0 };

	GMGLTechnique* glTechnique = gm_cast<GMGLTechnique*>(technique);
	IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
	const char* strIndex = number(index);

	combineUniform(light_Position, "GM_lights[", strIndex, "].Color");
	shaderProgram->setVec3(light_Position, d->color);

	combineUniform(light_Color, "GM_lights[", strIndex, "].Position");
	shaderProgram->setVec3(light_Color, d->position);

	combineUniform(light_Type, "GM_lights[", strIndex, "].Type");
	shaderProgram->setInt(light_Type, getType());

	combineUniform(light_Attenuation_Constant, "GM_lights[", strIndex, "].Attenuation.Constant");
	shaderProgram->setFloat(light_Attenuation_Constant, d->attenuation.constant);

	combineUniform(light_Attenuation_Linear, "GM_lights[", strIndex, "].Attenuation.Linear");
	shaderProgram->setFloat(light_Attenuation_Linear, d->attenuation.linear);

	combineUniform(light_Attenuation_Exp, "GM_lights[", strIndex, "].Attenuation.Exp");
	shaderProgram->setFloat(light_Attenuation_Exp, d->attenuation.exp);
}