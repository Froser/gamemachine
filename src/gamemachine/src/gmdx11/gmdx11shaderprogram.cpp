#include "stdafx.h"
#include "gmdx11shaderprogram.h"
#include <linearmath.h>

namespace
{
	inline const GMShaderVariablesDesc& GMGetDefaultShaderVariablesDesc()
	{
		static GMShaderVariablesDesc desc =
		{
			"WorldMatrix",
			"ViewMatrix",
			"ProjectionMatrix",
			"InverseTransposeModelMatrix",

			{ "OffsetX", "OffsetY", "ScaleX", "ScaleY", "Enabled" },
			"AmbientTextureAttributes",
			"DiffuseTextureAttributes",
			"NormalMapTextureAttributes",
			"LightmapTextureAttributes",
			"CubeMapTextureAttributes",

			{ "Position", "Color" },
			{ "AmbientLights", "AmbientLightCount" },
			{ "SpecularLights", "SpecularLightCount" },

			{ "Ka", "Kd", "Ks", "Shininess", "Refractivity" },
			"Material",

			"GMRasterizerState",
			"GMBlendState",
			"GMDepthStencilState",
		};
		return desc;
	}
}

GMDx11EffectShaderProgram::GMDx11EffectShaderProgram(GMComPtr<ID3DX11Effect> effect)
{
	D(d);
	d->effect = effect;
}

void GMDx11EffectShaderProgram::useProgram()
{
}

void GMDx11EffectShaderProgram::setMatrix4(const char* name, const GMMat4& value)
{
	D(d);
	ID3DX11EffectMatrixVariable* var = d->effect->GetVariableByName(name)->AsMatrix();
	GM_DX_HR(var->SetMatrix(ValuePointer(value)));
}

void GMDx11EffectShaderProgram::setVec4(const char* name, const GMFloat4& vector)
{
	D(d);
	ID3DX11EffectVectorVariable* var = d->effect->GetVariableByName(name)->AsVector();
	GM_DX_HR(var->SetFloatVector(ValuePointer(vector)));
}

void GMDx11EffectShaderProgram::setVec3(const char* name, const GMfloat value[3])
{
	D(d);
	GM_ASSERT(false); //not supported
}

void GMDx11EffectShaderProgram::setInt(const char* name, GMint value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = d->effect->GetVariableByName(name)->AsScalar();
	GM_DX_HR(var->SetInt(value));
}

void GMDx11EffectShaderProgram::setFloat(const char* name, GMfloat value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = d->effect->GetVariableByName(name)->AsScalar();
	GM_DX_HR(var->SetFloat(value));
}

void GMDx11EffectShaderProgram::setBool(const char* name, bool value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = d->effect->GetVariableByName(name)->AsScalar();
	GM_DX_HR(var->SetBool(value));
}

bool GMDx11EffectShaderProgram::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMDx11EffectShaderProgram::getInterface(GameMachineInterfaceID id, void** out)
{
	D(d);
	if (id == GameMachineInterfaceID::D3D11Effect)
	{
		d->effect->AddRef();
		(*out) = d->effect;
		return true;
	}
	return false;
}

const GMShaderVariablesDesc& GMDx11EffectShaderProgram::getDesc()
{
	return GMGetDefaultShaderVariablesDesc();
}