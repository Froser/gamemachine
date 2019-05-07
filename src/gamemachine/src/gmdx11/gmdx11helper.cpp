#include "stdafx.h"
#include "gmdx11helper.h"
#include <gmdxincludes.h>
#include "foundation/gamemachine.h"
#include <gmcom.h>
#include "gmdx11shaderprogram.h"
#include "gmdx11fxc.h"

namespace
{
	IShaderProgram* newShaderProgram(IGraphicEngine* engine, ID3DX11Effect* effect)
	{
		IShaderProgram* shaderProgram = new GMDx11EffectShaderProgram(effect);
		bool b = engine->setInterface(GameMachineInterfaceID::D3D11ShaderProgram, shaderProgram);
		GM_ASSERT(b);
		return shaderProgram;
	}
}

IShaderProgram* GMDx11Helper::loadEffectShader(
	IGraphicEngine* engine,
	const GMString& code,
	const GMString& filepath,
	const GMString& gfxCandidate
)
{
	GMComPtr<ID3D10Blob> shaderBuffer;
	GMComPtr<ID3DX11Effect> effect;
	GMComPtr<ID3D11Device> device;

	// 初始化
	bool b = engine->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	if (!b || !device)
		return nullptr;

	// 先检查prefetch下是否有预编译的fx文件
	GMDx11FXC fxc;
	if (!gfxCandidate.isEmpty())
	{
		GMBuffer gfxBuffer;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Prefetch, gfxCandidate, &gfxBuffer);

		if (fxc.canLoad(code, gfxBuffer) && fxc.load(std::move(gfxBuffer), device, &effect))
			return newShaderProgram(engine, effect);
	}

	// 如果没有prefetch，则查找是否有前一次生成的prefetch文件
	GMDx11FXCDescription desc;
	desc.code = code;
#if !GM_DEBUG
	desc.debug = false;
#endif

	if (fxc.tryLoadCache(desc, device, &effect))
		return newShaderProgram(engine, effect);

	// 编译一次代码
	GMComPtr<ID3D10Blob> errorMessage;
	if (fxc.compile(desc, &shaderBuffer, &errorMessage))
	{
		if (fxc.tryLoadCache(desc, device, &effect))
			return newShaderProgram(engine, effect);
	}
	else
	{
		GMStringReader reader(code);
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
			gm_error(gm_dbg_wrap("Cannot find shader file {0}"), filepath);
			GM_ASSERT(false);
		}
	}
	return nullptr;
}

void GMDx11Helper::loadShader(
	const IRenderContext* context,
	const GMString& effectFilePath,
	const GMString& gfxCandidate
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
	tech.setPrefetch(GMRenderEnvironment::DirectX11, gfxCandidate);
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