#include "stdafx.h"
#include "gmdx11shaderprogram.h"
#include <linearmath.h>

GMDx11EffectShaderProgram::GMDx11EffectShaderProgram(GMComPtr<ID3DX11Effect> effect)
{
	D(d);
	d->effect = effect;
	d->nextVariableIndex.exchange(0);
}

void GMDx11EffectShaderProgram::useProgram()
{
}

GMint32 GMDx11EffectShaderProgram::getIndex(const GMString& name)
{
	D(d);
	ID3DX11EffectVariable* var = d->effect->GetVariableByName(name.toStdString().c_str());
	if (var && var->IsValid())
	{
		d->variables.push_back(var);
		++d->nextVariableIndex;
		GM_ASSERT(d->variables.size() == d->nextVariableIndex);
		return d->nextVariableIndex - 1;
	}
	else
	{
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Wrong effect variable type"));
		return -1; //wrong type
	}
}

void GMDx11EffectShaderProgram::setMatrix4(GMint32 index, const GMMat4& value)
{
	D(d);
	ID3DX11EffectMatrixVariable* var = getMatrixVariable(index);
	GM_DX_HR(var->SetMatrix(ValuePointer(value)));
}

void GMDx11EffectShaderProgram::setVec4(GMint32 index, const GMFloat4& vector)
{
	D(d);
	ID3DX11EffectVectorVariable* var = getVectorVariable(index);
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
	ID3DX11EffectScalarVariable* var = getScalarVariable(index);
	GM_DX_HR(var->SetInt(value));
}

void GMDx11EffectShaderProgram::setFloat(GMint32 index, GMfloat value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = getScalarVariable(index);
	GM_DX_HR(var->SetFloat(value));
}

void GMDx11EffectShaderProgram::setBool(GMint32 index, bool value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = getScalarVariable(index);
	GM_DX_HR(var->SetBool(value));
}

bool GMDx11EffectShaderProgram::setInterfaceInstance(const GMString& interfaceName, const GMString& instanceName, GMShaderType type)
{
	D(d);
	ID3DX11EffectInterfaceVariable* interfaceVariable = getInterfaceVariable(getIndex(interfaceName));
	ID3DX11EffectClassInstanceVariable* instanceVariable = getInstanceVariable(getIndex(instanceName));
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

ID3DX11EffectVectorVariable* GMDx11EffectShaderProgram::getVectorVariable(GMint32 index)
{
	D(d);
	ID3DX11EffectVectorVariable* var = d->variables[index]->AsVector();
	GM_ASSERT(var->IsValid());
	return var;
}

ID3DX11EffectMatrixVariable* GMDx11EffectShaderProgram::getMatrixVariable(GMint32 index)
{
	D(d);
	ID3DX11EffectMatrixVariable* var = d->variables[index]->AsMatrix();
	GM_ASSERT(var->IsValid());
	return var;
}

ID3DX11EffectScalarVariable* GMDx11EffectShaderProgram::getScalarVariable(GMint32 index)
{
	D(d);
	ID3DX11EffectScalarVariable* var = d->variables[index]->AsScalar();
	GM_ASSERT(var->IsValid());
	return var;
}

ID3DX11EffectInterfaceVariable* GMDx11EffectShaderProgram::getInterfaceVariable(GMint32 index)
{
	D(d);
	ID3DX11EffectInterfaceVariable* var = d->variables[index]->AsInterface();
	GM_ASSERT(var->IsValid());
	return var;
}

ID3DX11EffectClassInstanceVariable* GMDx11EffectShaderProgram::getInstanceVariable(GMint32 index)
{
	D(d);
	ID3DX11EffectClassInstanceVariable* var = d->variables[index]->AsClassInstance();
	GM_ASSERT(var->IsValid());
	return var;
}