#include "stdafx.h"
#include "gmdx11helper.h"
#include <gmdxincludes.h>
#include "foundation/gamemachine.h"
#include <gmcom.h>
#include "gmdx11shaderprogram.h"

IShaderProgram* GMDx11Helper::loadEffectShader(
	IGraphicEngine* engine,
	const GMString& code,
	const GMString& filepath
)
{
	GMComPtr<ID3D10Blob> errorMessage;
	GMComPtr<ID3D10Blob> shaderBuffer;
	HRESULT hr;

	if (code.isEmpty())
		return nullptr;

	UINT flag = D3DCOMPILE_ENABLE_STRICTNESS;
#if GM_DEBUG
	flag |= D3DCOMPILEGM_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	std::string stdCode = code.toStdString();
	hr = D3DX11CompileFromMemory(
		stdCode.c_str(),
		stdCode.length(),
		filepath.toStdString().c_str(),
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
			char* t = ((char*)ptr);
			gm_error(gm_dbg_wrap("Error in Buf: {0}"), t);
			GM_ASSERT(false);
		}
		else
		{
			gm_error(gm_dbg_wrap("Cannot find shader file {0}"), filepath);
			GM_ASSERT(false);
		}
		return nullptr;
	}

	GMComPtr<ID3D11Device> device;
	bool b = engine->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	if (!b || !device)
		return nullptr;

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

void GMDx11Helper::loadShader(
	const IRenderContext* context,
	const GMString& effectFilePath
)
{
	GMBuffer buf;
	GMString path;
	GM.getGamePackageManager()->readFile(GMPackageIndex::Shaders, effectFilePath, &buf, &path);
	buf.convertToStringBuffer();

	GMRenderTechniques techs;
	GMRenderTechnique tech(GMShaderType::Effect);
	tech.setCode(GMRenderEnvironment::DirectX11, GMString(reinterpret_cast<const char*>(buf.buffer)));
	tech.setPath(GMRenderEnvironment::DirectX11, path);
	techs.addRenderTechnique(tech);
	context->getEngine()->getRenderTechniqueManager()->addRenderTechniques(techs);
}

const D3D11_SAMPLER_DESC& GMDx11Helper::getDefaultSamplerDesc()
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

D3D11_FILTER GMDx11Helper::getTextureFilter(GMS_TextureFilter min, GMS_TextureFilter mag)
{
	//TODO 缺少mip过滤
	if (min == GMS_TextureFilter::Linear && mag == GMS_TextureFilter::Linear)
		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	if (min == GMS_TextureFilter::Nearest && mag == GMS_TextureFilter::Nearest)
		return D3D11_FILTER_MIN_MAG_MIP_POINT;

	//TODO
	return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
}
