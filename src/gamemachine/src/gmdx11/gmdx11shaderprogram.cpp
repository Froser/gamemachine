#include "stdafx.h"
#include "gmdx11shaderprogram.h"
#include <linearmath.h>

GMDx11EffectShaderProgram::GMDx11EffectShaderProgram(GMComPtr<ID3DX11Effect> effect)
{
	D(d);
	d->effect = effect;
}

void GMDx11EffectShaderProgram::useProgram()
{
}

void GMDx11EffectShaderProgram::setMatrix4(GMint32 index, const GMMat4& value)
{
	D(d);
	ID3DX11EffectMatrixVariable* var = getMatrixVariable(name);
	GM_DX_HR(var->SetMatrix(ValuePointer(value)));
}

void GMDx11EffectShaderProgram::setVec4(GMint32 index, const GMFloat4& vector)
{
	D(d);
	ID3DX11EffectVectorVariable* var = getVectorVariable(name);
	GM_DX_HR(var->SetFloatVector(ValuePointer(vector)));
}

void GMDx11EffectShaderProgram::setVec3(GMint32 index, const GMfloat value[3])
{
	D(d);
	GM_ASSERT(false); //not supported
}

void GMDx11EffectShaderProgram::setInt(GMint32 index, GMint32 value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = getScalarVariable(name);
	GM_DX_HR(var->SetInt(value));
}

void GMDx11EffectShaderProgram::setFloat(GMint32 index, GMfloat value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = getScalarVariable(name);
	GM_DX_HR(var->SetFloat(value));
}

void GMDx11EffectShaderProgram::setBool(GMint32 index, bool value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = getScalarVariable(name);
	GM_DX_HR(var->SetBool(value));
}

bool GMDx11EffectShaderProgram::setInterfaceInstance(const GMString& interfaceName, const GMString& instanceName, GMShaderType type)
{
	D(d);
	ID3DX11EffectInterfaceVariable* interfaceVariable = getInterfaceVariable(interfaceName);
	ID3DX11EffectClassInstanceVariable* instanceVariable = getInstanceVariable(instanceName);
	if (instanceVariable->IsValid())
	{
		GM_DX_HR_RET(interfaceVariable->SetClassInstance(instanceVariable));
	}
	else
	{
		return false;
	}
	return true;
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

ID3DX11EffectVectorVariable* GMDx11EffectShaderProgram::getVectorVariable(const GMString& name)
{
	D(d);
	auto& container = d->vectors;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectVectorVariable* var = d->effect->GetVariableByName(name.toStdString().c_str())->AsVector();
		container[name] = var;
		return var;
	}
	return iter->second;
}

ID3DX11EffectMatrixVariable* GMDx11EffectShaderProgram::getMatrixVariable(const GMString& name)
{
	D(d);
	auto& container = d->matrices;
	auto iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectMatrixVariable* var = d->effect->GetVariableByName(name.toStdString().c_str())->AsMatrix();
		container[name] = var;
		return var;
	}
	return iter->second;
}

ID3DX11EffectScalarVariable* GMDx11EffectShaderProgram::getScalarVariable(const GMString& name)
{
	D(d);
	auto& container = d->scalars;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectScalarVariable* var = d->effect->GetVariableByName(name.toStdString().c_str())->AsScalar();
		container[name] = var;
		return var;
	}
	return iter->second;
}

ID3DX11EffectInterfaceVariable* GMDx11EffectShaderProgram::getInterfaceVariable(const GMString& name)
{
	D(d);
	auto& container = d->interfaces;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectInterfaceVariable* var = d->effect->GetVariableByName(name.toStdString().c_str())->AsInterface();
		container[name] = var;
		return var;
	}
	return iter->second;
}

ID3DX11EffectClassInstanceVariable* GMDx11EffectShaderProgram::getInstanceVariable(const GMString& name)
{
	D(d);
	auto& container = d->instances;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectClassInstanceVariable* var = d->effect->GetVariableByName(name.toStdString().c_str())->AsClassInstance();
		container[name] = var;
		return var;
	}
	return iter->second;
}