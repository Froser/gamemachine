#include "stdafx.h"
#include "gmgllight.h"
#include "gmgltechniques.h"
#include "shader_constants.h"
#include "gmengine/gmlight_p.h"

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGLLight)
{
	struct LightIndices
	{
		GMint32 Color;
		GMint32 Position;
		GMint32 Type;
		GMint32 AttenuationConstant;
		GMint32 AttenuationLinear;
		GMint32 AttenuationExp;
		GMint32 AmbientIntensity;
		GMint32 DiffuseIntensity;
		GMint32 SpecularIntensity;
	};
	Vector<Vector<LightIndices>> lightIndices;
};

GMGLLight::GMGLLight()
{
	GM_CREATE_DATA();
}

GMGLLight::~GMGLLight()
{

}

void GMGLLight::activateLight(GMuint32 index, ITechnique* technique)
{
	D(d);
	D_BASE(db, Base);

	GMGLTechnique* glTechnique = gm_cast<GMGLTechnique*>(technique);
	IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
	GMString strIndex = GMString((GMint32)index);

	GMsize_t shaderLightIdx = verifyIndicesContainer(d->lightIndices, shaderProgram);
	static const Data::LightIndices s_empty = { 0 };
	if (d->lightIndices[shaderLightIdx].size() <= index)
		d->lightIndices[shaderLightIdx].resize(index + 1, s_empty);

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].Color, L"GM_lights[" + strIndex + L"].Color"),
		db->color);

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].Position, L"GM_lights[" + strIndex + L"].Position"),
		db->position);

	shaderProgram->setInt(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].Type, L"GM_lights[" + strIndex + L"].Type"),
		getLightType());

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].AmbientIntensity, L"GM_lights[" + strIndex + L"].AmbientIntensity"),
		db->ambientIntensity);

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].DiffuseIntensity, L"GM_lights[" + strIndex + L"].DiffuseIntensity"),
		db->diffuseIntensity);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].SpecularIntensity, L"GM_lights[" + strIndex + L"].SpecularIntensity"),
		db->specularIntensity);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].AttenuationConstant, L"GM_lights[" + strIndex + L"].Attenuation.Constant"),
		db->attenuation.constant);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].AttenuationLinear, L"GM_lights[" + strIndex + L"].Attenuation.Linear"),
		db->attenuation.linear);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].AttenuationExp, L"GM_lights[" + strIndex + L"].Attenuation.Exp"),
		db->attenuation.exp);
}

GM_PRIVATE_OBJECT_UNALIGNED_FROM(GMGLDirectionalLight, GMDirectionalLight_t)
{
	struct LightIndices
	{
		GMint32 Direction;
	};

	Vector<Vector<LightIndices>> lightIndices;
};

GMGLDirectionalLight::GMGLDirectionalLight()
{
	GM_CREATE_DATA();
}

GMGLDirectionalLight::~GMGLDirectionalLight()
{

}

bool GMGLDirectionalLight::setLightAttribute3(GMLightAttribute attr, GMfloat value[3])
{
	D(d);
	switch (attr)
	{
	case Direction:
		d->direction[0] = value[0];
		d->direction[1] = value[1];
		d->direction[2] = value[2];
		d->direction[3] = 1;
		break;
	default:
		return Base::setLightAttribute3(attr, value);
	}
	return true;
}

void GMGLDirectionalLight::activateLight(GMuint32 index, ITechnique* technique)
{
	Base::activateLight(index, technique);

	D(d);
	GMGLTechnique* glTechnique = gm_cast<GMGLTechnique*>(technique);
	IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
	GMString strIndex = GMString((GMint32)index);

	GMsize_t shaderLightIdx = verifyIndicesContainer(d->lightIndices, shaderProgram);
	static const Data::LightIndices s_empty = { 0 };
	if (d->lightIndices[shaderLightIdx].size() <= index)
		d->lightIndices[shaderLightIdx].resize(index + 1, s_empty);

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].Direction, L"GM_lights[" + strIndex + L"].Direction"),
		d->direction);
}

GM_PRIVATE_OBJECT_UNALIGNED_FROM(GMGLSpotlight, GMSpotlight_t)
{
	struct LightIndices
	{
		GMint32 CutOff;
	};

	Vector<Vector<LightIndices>> lightIndices;
};

GMGLSpotlight::GMGLSpotlight()
{
	GM_CREATE_DATA();
}

GMGLSpotlight::~GMGLSpotlight()
{

}

bool GMGLSpotlight::setLightAttribute(GMLightAttribute attr, GMfloat value)
{
	D(d);
	switch (attr)
	{
	case CutOff:
		d->cutOff = value;
		break;
	default:
		return Base::setLightAttribute(attr, value);
	}
	return true;
}

void GMGLSpotlight::activateLight(GMuint32 index, ITechnique* technique)
{
	Base::activateLight(index, technique);
	
	D(d);
	GMGLTechnique* glTechnique = gm_cast<GMGLTechnique*>(technique);
	IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
	GMString strIndex = GMString((GMint32)index);

	GMsize_t shaderLightIdx = verifyIndicesContainer(d->lightIndices, shaderProgram);
	static const Data::LightIndices s_empty = { 0 };
	if (d->lightIndices[shaderLightIdx].size() <= index)
		d->lightIndices[shaderLightIdx].resize(index + 1, s_empty);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].CutOff, L"GM_lights[" + strIndex + L"].CutOff"),
		Cos(Radians(d->cutOff)));
}

END_NS