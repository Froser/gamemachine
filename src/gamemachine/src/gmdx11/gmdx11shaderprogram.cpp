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

namespace
{
	nullptr_t* getNullPtr()
	{
		static nullptr_t s_null[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1] = { 0 };
		return s_null;
	}
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
	dc->CSSetShader(d->shader, NULL, 0);
	dc->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

	// 清理现场
	cleanUp();
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
	std::string stdEntryPoint = entryPoint.toStdString();
	HRESULT hr = D3DX11CompileFromMemory(
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
	);
	if (FAILED(hr))
	{
		GMStringReader reader(source);
		std::string report;
		GMint32 ln = 0;
		auto iter = reader.lineBegin();
		do
		{
			report += std::to_string(++ln) + ":\t" + (*iter).toStdString();
			iter++;
		} while (iter.hasNextLine());

		gm_error(gm_dbg_wrap("Shader source: \n{0}"), report);
		if (errorMessage)
		{
			void* ptr = errorMessage->GetBufferPointer();
			char* t = ((char*)ptr);
			gm_error(gm_dbg_wrap("Error in Buf: {0}"), t);
			GM_ASSERT(false);
		}
		else
		{
			gm_error(gm_dbg_wrap("Cannot find shader file {0}"), stdPath);
			GM_ASSERT(false);
		}
	}
	else
	{
		ID3D11Device* device = d->engine->getDevice();
		GM_DX_HR(device->CreateComputeShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, &d->shader));
	}
}


bool GMDx11ComputeShaderProgram::createReadOnlyBufferFrom(GMComputeBufferHandle bufferSrc, OUT GMComputeBufferHandle* bufferOut)
{
	D(d);
	if (bufferOut)
	{
		ID3D11Device* device = d->engine->getDevice();
		ID3D11DeviceContext* dc = d->engine->getDeviceContext();
		ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(bufferSrc);
		ID3D11Buffer* newBuffer = nullptr;

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		buffer->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		HRESULT hr = device->CreateBuffer(&desc, NULL, &newBuffer);
		*bufferOut = newBuffer;
		return SUCCEEDED(hr);
	}
	return false;
}

bool GMDx11ComputeShaderProgram::createBuffer(GMuint32 elementSize, GMuint32 count, void* pInitData, GMComputeBufferType type, OUT GMComputeBufferHandle* ppBufOut)
{
	D(d);
	ID3D11Device* device = d->engine->getDevice();

	*ppBufOut = NULL;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	if (type == GMComputeBufferType::Structured)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	}
	else
	{
		GM_ASSERT(type == GMComputeBufferType::Constant);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.MiscFlags = 0;
	}

	desc.ByteWidth = elementSize * count;
	desc.StructureByteStride = elementSize;

	if (pInitData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		GM_DX_HR_RET(device->CreateBuffer(&desc, &InitData, (ID3D11Buffer**)ppBufOut));
	}
	else
	{
		GM_DX_HR_RET(device->CreateBuffer(&desc, NULL, (ID3D11Buffer**)ppBufOut));
	}
	return true;
}

void GMDx11ComputeShaderProgram::release(GMComputeBufferHandle h)
{
	if (h)
	{
		ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(h);
		buffer->Release();
	}
}

bool GMDx11ComputeShaderProgram::createBufferShaderResourceView(GMComputeBufferHandle hBuffer, OUT GMComputeSRVHandle* out)
{
	D(d);
	ID3D11Device* device = d->engine->getDevice();
	ID3D11Buffer* pBuffer = static_cast<ID3D11Buffer*>(hBuffer);
	D3D11_BUFFER_DESC descBuf;
	GM_ZeroMemory(&descBuf, sizeof(descBuf));
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	GM_ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// Raw Buffer
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
	}
	else
	{
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// Structured Buffer
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			return false;
		}
	}

	return SUCCEEDED( device->CreateShaderResourceView(pBuffer, &desc, (ID3D11ShaderResourceView**)out) );
}

bool GMDx11ComputeShaderProgram::createBufferUnorderedAccessView(GMComputeBufferHandle hBuffer, OUT GMComputeUAVHandle* out)
{
	D(d);
	ID3D11Device* device = d->engine->getDevice();
	ID3D11Buffer* pBuffer = static_cast<ID3D11Buffer*>(hBuffer);
	D3D11_BUFFER_DESC descBuf;
	GM_ZeroMemory(&descBuf, sizeof(descBuf));
	pBuffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	GM_ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// Raw Buffer
		desc.Format = DXGI_FORMAT_R32_TYPELESS; // 创建Raw Unordered Access View的Format一定要为 DXGI_FORMAT_R32_TYPELESS
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc.Buffer.NumElements = descBuf.ByteWidth / 4;
	}
	else
	{
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// Structured Buffer
			desc.Format = DXGI_FORMAT_UNKNOWN; // 创建Structured Buffer的Raw Unordered Access View的Format一定要为 DXGI_FORMAT_UNKNOWN
			desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			return false;
		}
	}

	return SUCCEEDED( device->CreateUnorderedAccessView(pBuffer, &desc, (ID3D11UnorderedAccessView**)out) );
}

void GMDx11ComputeShaderProgram::setShaderResourceView(GMuint32 num, GMComputeSRVHandle* hs)
{
	D(d);
	if (num > 0)
	{
		ID3D11DeviceContext* dc = d->engine->getDeviceContext();
		dc->CSSetShaderResources(d->SRV_UAV_CB_count[0], num, (ID3D11ShaderResourceView**)hs);
		d->SRV_UAV_CB_count[0] += num;
	}
}

void GMDx11ComputeShaderProgram::setUnorderedAccessView(GMuint32 num, GMComputeUAVHandle* hs)
{
	D(d);
	if (num > 0)
	{
		ID3D11DeviceContext* dc = d->engine->getDeviceContext();
		dc->CSSetUnorderedAccessViews(d->SRV_UAV_CB_count[1], num, (ID3D11UnorderedAccessView**)hs, NULL);
		d->SRV_UAV_CB_count[1] += num;
	}
}

void GMDx11ComputeShaderProgram::setConstantBuffer(GMComputeBufferHandle handle, void* dataPtr, GMuint32 sizeInBytes)
{
	D(d);
	if (handle)
	{
		ID3D11DeviceContext* dc = d->engine->getDeviceContext();
		ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(handle);
		D3D11_MAPPED_SUBRESOURCE mappedResource = { 0 };
		GM_DX_HR(dc->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		memcpy(mappedResource.pData, dataPtr, sizeInBytes);
		dc->Unmap(buffer, 0);
		ID3D11Buffer* ppCB[1] = { buffer };
		dc->CSSetConstantBuffers(d->SRV_UAV_CB_count[2], 1, ppCB);
		++d->SRV_UAV_CB_count[2];
	}
}

void GMDx11ComputeShaderProgram::copyBuffer(GMComputeBufferHandle dest, GMComputeBufferHandle src)
{
	D(d);
	ID3D11DeviceContext* dc = d->engine->getDeviceContext();
	dc->CopyResource( static_cast<ID3D11Buffer*>(dest), static_cast<ID3D11Buffer*>(src));
}

void* GMDx11ComputeShaderProgram::mapBuffer(GMComputeBufferHandle handle)
{
	D(d);
	ID3D11DeviceContext* dc = d->engine->getDeviceContext();
	ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(handle);
	D3D11_MAPPED_SUBRESOURCE mappedResource = { 0 };
	GM_DX_HR(dc->Map(buffer, 0, D3D11_MAP_READ, 0, &mappedResource));
	return mappedResource.pData;
}

void GMDx11ComputeShaderProgram::unmapBuffer(GMComputeBufferHandle handle)
{
	D(d);
	ID3D11DeviceContext* dc = d->engine->getDeviceContext();
	ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(handle);
	dc->Unmap(buffer, 0);
}


bool GMDx11ComputeShaderProgram::canRead(GMComputeBufferHandle handle)
{
	ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(handle);
	D3D11_BUFFER_DESC desc = { 0 };
	buffer->GetDesc(&desc);
	return !!(desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ);
}

bool GMDx11ComputeShaderProgram::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

void GMDx11ComputeShaderProgram::cleanUp()
{
	D(d);
	ID3D11DeviceContext* dc = d->engine->getDeviceContext();
	dc->CSSetShader(NULL, NULL, 0);
	nullptr_t* nullpointers = getNullPtr();
	if (d->SRV_UAV_CB_count[0])
	{
		dc->CSSetShaderResources(0, d->SRV_UAV_CB_count[0], (ID3D11ShaderResourceView**)nullpointers);
		d->SRV_UAV_CB_count[0] = 0;
	}

	if (d->SRV_UAV_CB_count[1])
	{
		dc->CSSetUnorderedAccessViews(0, d->SRV_UAV_CB_count[1], (ID3D11UnorderedAccessView**)nullpointers, NULL);
		d->SRV_UAV_CB_count[1] = 0;
	}

	if (d->SRV_UAV_CB_count[2])
	{
		dc->CSSetConstantBuffers(0, d->SRV_UAV_CB_count[2], (ID3D11Buffer**)nullpointers);
		d->SRV_UAV_CB_count[2] = 0;
	}

	GM_ZeroMemory(d->SRV_UAV_CB_count, sizeof(d->SRV_UAV_CB_count));
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
	default:
		break;
	}
	return false;
}
