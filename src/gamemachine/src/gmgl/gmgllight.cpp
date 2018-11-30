#include "stdafx.h"
#include "gmgllight.h"
#include "gmgltechniques.h"
#include "shader_constants.h"

template <GMuint32 i>
struct String {};

namespace
{
	const GMString& number(GMint32 index)
	{
		static GMString arr[10] = {
			L"0",
			L"1",
			L"2",
			L"3",
			L"4",
			L"5",
			L"6",
			L"7",
			L"8",
			L"9",
		};
		return arr[index];
	}
}

void GMGLLight::activateLight(GMuint32 index, ITechnique* technique)
{
	D(d);
	static GMString light_Position;
	static GMString light_Color;
	static GMString light_Type;
	static GMString light_Attenuation_Constant;
	static GMString light_Attenuation_Linear;
	static GMString light_Attenuation_Exp;

	GMGLTechnique* glTechnique = gm_cast<GMGLTechnique*>(technique);
	IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
	GMString strIndex = number(index);

	light_Position = L"GM_lights[" + strIndex + L"].Color";
	shaderProgram->setVec3(light_Position, d->color);

	light_Color = L"GM_lights[" + strIndex + L"].Position";
	shaderProgram->setVec3(light_Color, d->position);

	light_Type = L"GM_lights[" + strIndex + L"].Type";
	shaderProgram->setInt(light_Type, getType());

	light_Attenuation_Constant = L"GM_lights[" + strIndex + L"].Attenuation.Constant";
	shaderProgram->setFloat(light_Attenuation_Constant, d->attenuation.constant);

	light_Attenuation_Linear = L"GM_lights[" + strIndex + L"].Attenuation.Linear";
	shaderProgram->setFloat(light_Attenuation_Linear, d->attenuation.linear);

	light_Attenuation_Exp = L"GM_lights[" + strIndex = L"].Attenuation.Exp";
	shaderProgram->setFloat(light_Attenuation_Exp, d->attenuation.exp);
}