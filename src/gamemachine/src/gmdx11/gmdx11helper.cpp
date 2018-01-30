#include "stdafx.h"
#include "gmdx11helper.h"
#include <gmdxincludes.h>
#include "foundation/gamemachine.h"
#include <gmcom.h>

extern "C"
{
	HRESULT GMLoadDx11Shader(
		const gm::GMString& filename,
		const gm::GMString& entryPoint,
		const gm::GMString& profile,
		GMShaderType type
	)
	{
		gm::GMComPtr<ID3D10Blob> errorMessage;
		gm::GMComPtr<ID3D10Blob> shaderBuffer;
		HRESULT hr;

		gm::GMBuffer buf, pixelBuf;
		gm::GMString path, pixelPath;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, filename, &buf, &path);
		if (!buf.size)
			return E_FAIL;

		buf.convertToStringBuffer();
		pixelBuf.convertToStringBuffer();

		hr = D3DX11CompileFromMemory(
			(char*)buf.buffer,
			buf.size,
			path.toStdString().c_str(),
			NULL,
			NULL,
			entryPoint.toStdString().c_str(),
			profile.toStdString().c_str(),
			D3D10_SHADER_ENABLE_STRICTNESS,
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
				gm_error(L"Error in Buf");
				GM_ASSERT(false);
			}
			else
			{
				gm_error(L"Cannot find shader file %s", path.c_str());
				GM_ASSERT(false);
			}
			return E_FAIL;
		}

		ID3D11Device* device = nullptr;
		IGraphicEngine* engine = GM.getGraphicEngine();
		bool b = GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
		if (!b || !device)
			return E_FAIL;

		if (type == GM_VERTEX_SHADER)
		{
			GMComPtr<ID3D11VertexShader> vertexShader;
			hr = device->CreateVertexShader(
				shaderBuffer->GetBufferPointer(),
				shaderBuffer->GetBufferSize(),
				NULL,
				&vertexShader
			);
			GM_COM_CHECK_RETURN(hr, hr);
			b = engine->setInterface(gm::GameMachineInterfaceID::D3D11VertexShader, vertexShader.get());
			if (!b)
				return E_FAIL;

			b = engine->setInterface(gm::GameMachineInterfaceID::D3D11VertexShaderBuffer, shaderBuffer.get());
			if (!b)
				return E_FAIL;
		}
		else
		{
			if (type != GM_PIXEL_SHADER)
			{
				GM_ASSERT(false);
				return E_FAIL;
			}

			GMComPtr<ID3D11PixelShader> pixelShader;
			hr = device->CreatePixelShader(
				shaderBuffer->GetBufferPointer(),
				shaderBuffer->GetBufferSize(),
				NULL,
				&pixelShader
			);
			GM_COM_CHECK_RETURN(hr, hr);
			b = engine->setInterface(gm::GameMachineInterfaceID::D3D11PixelShader, pixelShader.get());
			if (!b)
				return E_FAIL;

			b = engine->setInterface(gm::GameMachineInterfaceID::D3D11PixelShaderBuffer, shaderBuffer.get());
			if (!b)
				return E_FAIL;
		}

		return S_OK;
	}
}