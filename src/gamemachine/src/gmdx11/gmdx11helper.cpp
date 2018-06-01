#include "stdafx.h"
#include "gmdx11helper.h"
#include <gmdxincludes.h>
#include "foundation/gamemachine.h"
#include <gmcom.h>
#include "gmdx11shaderprogram.h"

IShaderProgram* GMDx11Helper::GMLoadDx11Shader(
	IGraphicEngine* engine,
	const gm::GMString& filename,
	GMShaderType type
)
{
	gm::GMComPtr<ID3D10Blob> errorMessage;
	gm::GMComPtr<ID3D10Blob> shaderBuffer;
	HRESULT hr;

	gm::GMBuffer buf;
	gm::GMString path;
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, filename, &buf, &path);
	if (!buf.size)
		return nullptr;

	UINT flag = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
	flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	buf.convertToStringBuffer();
	hr = D3DX11CompileFromMemory(
		(char*)buf.buffer,
		buf.size,
		path.toStdString().c_str(),
		NULL,
		NULL,
		"",
		"fx_5_0",
		flag,
		0,
		NULL,
		&shaderBuffer,
		&errorMessage,
		NULL
	);
	if (FAILED(hr))
	{
		if (errorMessage)
		{
			void* ptr = errorMessage->GetBufferPointer();
			size_t sz = errorMessage->GetBufferSize();
			char* t = ((char*)ptr);
			gm_error(L"Error in Buf: %s", t);
			GM_ASSERT(false);
		}
		else
		{
			gm_error(L"Cannot find shader file %s", path.c_str());
			GM_ASSERT(false);
		}
		return nullptr;
	}

	GMComPtr<ID3D11Device> device;
	bool b = engine->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	if (!b || !device)
		return nullptr;

	if (type == GMShaderType::Effect)
	{
		GMComPtr<ID3DX11Effect> effect;
		GM_DX_HR(D3DX11CreateEffectFromMemory(
			shaderBuffer->GetBufferPointer(),
			shaderBuffer->GetBufferSize(),
			0,
			device,
			&effect
		));

		GMDx11EffectShaderProgram* shaderProgram = new GMDx11EffectShaderProgram(effect);
		b = engine->setInterface(GameMachineInterfaceID::D3D11ShaderProgram, shaderProgram);
		GM_ASSERT(b);

		return shaderProgram;
	}
	else
	{
		return nullptr;
	}

	return nullptr;
}

const D3D11_SAMPLER_DESC& GMDx11Helper::GMGetDx11DefaultSamplerDesc()
{
	static D3D11_SAMPLER_DESC desc = {
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0,
		0,
		D3D11_COMPARISON_NEVER,
		0,
		D3D11_FLOAT32_MAX
	};
	return desc;
}

D3D11_FILTER GMDx11Helper::GMGetDx11Filter(GMS_TextureFilter min, GMS_TextureFilter mag)
{
	//TODO 缺少mip过滤
	if (min == GMS_TextureFilter::LINEAR && mag == GMS_TextureFilter::LINEAR)
		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	if (min == GMS_TextureFilter::NEAREST && mag == GMS_TextureFilter::NEAREST)
		return D3D11_FILTER_MIN_MAG_MIP_POINT;

	//TODO
	return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
}
