#include "stdafx.h"
#include "gmdx11shaderprogram.h"
#include <linearmath.h>
#include "gmdx11graphic_engine.h"

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
		return -1; //wrong type
	}
}

void GMDx11EffectShaderProgram::setMatrix4(GMint32 index, const GMMat4& value)
{
	D(d);
	if (index < 0)
		return;

	ID3DX11EffectMatrixVariable* var = getMatrixVariable(index);
	GM_DX_HR(var->SetMatrix(ValuePointer(value)));
}

void GMDx11EffectShaderProgram::setVec4(GMint32 index, const GMFloat4& vector)
{
	D(d);
	if (index < 0)
		return;

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
	if (index < 0)
		return;

	ID3DX11EffectScalarVariable* var = getScalarVariable(index);
	GM_DX_HR(var->SetInt(value));
}

void GMDx11EffectShaderProgram::setFloat(GMint32 index, GMfloat value)
{
	D(d);
	if (index < 0)
		return;

	ID3DX11EffectScalarVariable* var = getScalarVariable(index);
	GM_DX_HR(var->SetFloat(value));
}

void GMDx11EffectShaderProgram::setBool(GMint32 index, bool value)
{
	D(d);
	if (index < 0)
		return;

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
		if (out)
		{
			d->effect->AddRef();
			(*out) = d->effect;
			return true;
		}
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

GMDx11ComputeShaderProgram::GMDx11ComputeShaderProgram(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->engine = gm_cast<GMDx11GraphicEngine*>(d->context->getEngine());
}

void GMDx11ComputeShaderProgram::dispatch(GMint32 threadGroupCountX, GMint32 threadGroupCountY, GMint32 threadGroupCountZ)
{
	D(d);
	ID3D11DeviceContext* dc = d->engine->getDeviceContext();
	dc->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void GMDx11ComputeShaderProgram::load(const GMString& path, const GMString& source, const GMString& entryPoint)
{
	D(d);
	GMComPtr<ID3D10Blob> errorMessage;
	GMComPtr<ID3D10Blob> shaderBuffer;
	UINT flag = D3DCOMPILE_ENABLE_STRICTNESS;
#if GM_DEBUG
	flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	std::string stdCode = source.toStdString();
	std::string stdPath = path.toStdString();
	std::string stdEntryPoint = path.toStdString();
	GM_DX_HR(D3DX11CompileFromMemory(
		stdCode.c_str(),
		stdCode.length(),
		stdPath.c_str(),
		NULL,
		NULL,
		stdEntryPoint.c_str(),
		"cs_5_0",
		flag,
		0,
		NULL,
		&shaderBuffer,
		&errorMessage,
		NULL
	));

	ID3D11Device* device = d->engine->getDevice();
	device->CreateComputeShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, &d->shader);
}

bool GMDx11ComputeShaderProgram::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMDx11ComputeShaderProgram::getInterface(GameMachineInterfaceID id, void** out)
{
	D(d);
	switch (id)
	{
	case gm::GameMachineInterfaceID::D3D11ShaderProgram:
	{
		if (out)
		{
			d->shader->AddRef();
			(*out) = d->shader;
			return true;
		}
	}
	case gm::GameMachineInterfaceID::CustomInterfaceBegin:
		break;
	default:
		break;
	}
	return false;
}
