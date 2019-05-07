#include "stdafx.h"
#include "gmdx11fxc.h"
#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <strsafe.h>
#include <fstream>
#include <gmcom.h>
#include <foundation/gmcryptographic.h>

#define FXC_FILENAME L"main.gfx"
#define TEMP_CODE_FILE "code.hlsl"
#define LOG_FILE "log.txt"
#define BUFSIZE 10 * 1024

#define GM_HEADER "GameMachine_"
#define GM_HEADER_LEN 12
#define GM_FXC_VERSION_MAJOR 1
#define GM_FXC_VERSION_MINOR 0

namespace
{
	struct GMBlob : ID3DBlob
	{
		GMBlob(const GMBuffer& buffer)
			: buf(buffer)
			, refCount(1)
		{

		}

		GMBlob(GMBuffer&& buffer)
			: buf(std::move(buffer))
			, refCount(1)
		{

		}

		virtual LPVOID STDMETHODCALLTYPE GetBufferPointer(void)
		{
			return buf.buffer;
		}

		virtual SIZE_T STDMETHODCALLTYPE GetBufferSize(void)
		{
			return buf.size;
		}

		STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv)
		{
			if (IsEqualIID(iid, IID_IUnknown))
			{
				*ppv = (IUnknown *) this;
			}

			return E_NOINTERFACE;
		}

		ULONG AddRef()
		{
			return ++refCount;
		}

		ULONG Release()
		{
			if (--refCount > 0)
			{
				return refCount;
			}
			else
			{
				delete this;
			}

			return 0;
		}


	private:
		GMBuffer buf;
		GMint32 refCount;
	};

	struct Version
	{
		GMint32 major;
		GMint32 minor;
	};

	bool getFileHeaders(const GMBuffer& fxcBuffer, Version& version, GMbyte fingerprints[16], OUT GMsize_t& contentPtr)
	{
		GM_ASSERT(fxcBuffer.buffer);

		GMsize_t uOffset = 0;
		if (memcmp(fxcBuffer.buffer, GM_HEADER, GM_HEADER_LEN) != 0)
			return false;

		uOffset += GM_HEADER_LEN;
		memcpy_s(&version, sizeof(Version), fxcBuffer.buffer + uOffset, sizeof(Version));

		uOffset += sizeof(Version);
		memcpy_s(fingerprints, 16, fxcBuffer.buffer + uOffset, 16);

		contentPtr = uOffset + 16;
		return true;
	}

	void sourceCodeToBufferW(const GMString& code, REF GMBuffer& buffer)
	{
		buffer.size = code.length() * sizeof(GMwchar);
		buffer.buffer = (GMbyte*) code.c_str();
		buffer.needRelease = false;
	}

	void sourceCodeToBufferA(const std::string& code, REF GMBuffer& buffer)
	{
		buffer.size = code.length();
		buffer.buffer = (GMbyte*)code.c_str();
		buffer.needRelease = false;
	}
}

bool GMDx11FXC::canLoad(const GMString& code, const GMBuffer& fxcBuffer)
{
	if (!fxcBuffer.buffer)
		return false;

	// 检查Header
	Version version;
	GMbyte fileFingerprints[16];
	GMsize_t contentOffset = 0;
	if (!getFileHeaders(fxcBuffer, version, fileFingerprints, contentOffset))
	{
		gm_warning(gm_dbg_wrap("FXC wrong header."));
		return false;
	}

	GMBuffer sourceCode;
	sourceCodeToBufferW(code, sourceCode);
	GMBuffer md5;
	GMCryptographic::hash(sourceCode, GMCryptographic::MD5, md5);
	GM_ASSERT(md5.size == 16);

	if (memcmp(fileFingerprints, md5.buffer, md5.size) != 0)
	{
		gm_warning(gm_dbg_wrap("Shader code doesn't match fxc file."));
		return false;
	}

	return true;
}

bool GMDx11FXC::compile(GMDx11FXCDescription& desc, ID3DBlob** ppCode, ID3DBlob** ppErrorMessages)
{
	if (!fillDescription(&desc))
		return false;

	GMBuffer bufCode;
	std::string strCode = desc.code.toStdString();
	sourceCodeToBufferA(strCode, bufCode);

	UINT flags1 = D3DCOMPILE_ENABLE_STRICTNESS;
	if (desc.debug)
		flags1 |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	if (desc.treatWarningsAsErrors)
		flags1 |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
	if (desc.optimizationLevel == 0)
		flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
	else if (desc.optimizationLevel == 1)
		flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
	else if (desc.optimizationLevel == 2)
		flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
	else if (desc.optimizationLevel == 3)
		flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
	else
	{
		flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
		gm_warning(gm_dbg_wrap("Unrecognized optimization level. Use default."));
	}

	gm_info(gm_dbg_wrap("Compiling HLSL code..."));
	if (SUCCEEDED(D3DCompile(
		bufCode.buffer,
		bufCode.size,
		NULL,
		NULL,
		0,
		"",
		"fx_5_0",
		flags1,
		0,
		ppCode,
		ppErrorMessages
	)))
	{
		return makeFingerprints(desc, *ppCode);
	}

	return false;
}

bool GMDx11FXC::load(const GMBuffer& shaderBuffer, ID3D11Device* pDevice, ID3DX11Effect** ppEffect)
{
	Version version;
	GMbyte fingerprints[16];
	GMsize_t uOffset = 0;
	getFileHeaders(shaderBuffer, version, fingerprints, uOffset);
	if (version.major == 1 && version.minor == 0)
	{
		GMBufferView gfxView (shaderBuffer, uOffset);
		GMComPtr<ID3DBlob> shaderBufferBlob = new GMBlob(gfxView);
		return SUCCEEDED(D3DX11CreateEffectFromMemory(
			shaderBufferBlob->GetBufferPointer(),
			shaderBufferBlob->GetBufferSize(),
			0,
			pDevice,
			ppEffect
		));
	}

	gm_warning(gm_dbg_wrap("The version {0}.{1} is not supported."), GMString(version.major), GMString(version.minor));
	return false;
}

bool GMDx11FXC::tryLoadCache(GMDx11FXCDescription& desc, ID3D11Device* pDevice, ID3DX11Effect** ppEffect)
{
	if (!fillDescription(&desc))
		return false;

	GMString outputPath = desc.fxcOutputDir + L"\\" FXC_FILENAME;
	HANDLE hFile = CreateFile(outputPath.c_str(), GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	GMBuffer buf;
	DWORD dwSize = GetFileSize(hFile, NULL);
	buf.buffer = new GMbyte[dwSize];
	buf.size = dwSize;
	buf.needRelease = true;
	ReadFile(hFile, buf.buffer, (DWORD) buf.size, NULL, NULL);
	CloseHandle(hFile);

	return (canLoad(desc.code, buf) && load(buf, pDevice, ppEffect));
}

bool GMDx11FXC::fillDescription(GMDx11FXCDescription* desc)
{
	GM_ASSERT(desc);
	if (desc->fxcOutputDir.isEmpty())
	{
		TCHAR szTempPath[MAX_PATH];
		GetModuleFileNameW(NULL, szTempPath, MAX_PATH);
		PathRemoveFileSpec(szTempPath);
		PathCombine(szTempPath, szTempPath, _T("prefetch"));
		desc->fxcOutputDir = GMString(szTempPath);
		if (!PathFileExists(szTempPath) && !CreateDirectory(szTempPath, NULL))
			return false;
	}

	return true;
}

bool GMDx11FXC::makeFingerprints(const GMDx11FXCDescription& desc, ID3DBlob* pCode)
{
	TCHAR fxcOutput[MAX_PATH];
	PathCombine(fxcOutput, desc.fxcOutputDir.c_str(), _T("main.gfx"));

	// 再次打开生成文件，创建带有GM标识头的fxc文件，在文件头加上GameMachine {版本号(int32)}{代码MD5}。
	HANDLE hFile = CreateFile(fxcOutput, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	GMBuffer sourceCode;
	sourceCodeToBufferW(desc.code, sourceCode);

	GMBuffer md5;
	GMCryptographic::hash(sourceCode, GMCryptographic::MD5, md5);

	// 准备一块空间content，这段缓存就是最终文件的内容
	GM_ASSERT(md5.size == 16);
	constexpr GMsize_t HEADER_LEN = GM_HEADER_LEN + sizeof(Version) + 16; //16表示16字节的MD5
	GMBuffer header;
	header.size = HEADER_LEN;
	header.buffer = new GMbyte[header.size];

	// 写 "GameMachine"
	GMsize_t uOffset = GM_HEADER_LEN;
	memcpy_s(header.buffer, header.size, GM_HEADER, uOffset);

	// 写 版本号 Major Minor
	Version version = {
		GM_FXC_VERSION_MAJOR,
		GM_FXC_VERSION_MINOR,
	};
	memcpy_s(header.buffer + uOffset, header.size - uOffset, &version, sizeof(Version));
	uOffset += sizeof(Version);

	// 写MD5
	memcpy_s(header.buffer + uOffset, header.size - uOffset, md5.buffer, md5.size);
	uOffset += md5.size;

	// 获取编译好的二进制代码
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if (!WriteFile(hFile, header.buffer, (DWORD) header.size, NULL, NULL))
		return false;

	if (!WriteFile(hFile, pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, NULL))
		return false;

	SetEndOfFile(hFile);
	CloseHandle(hFile);
	return true;
}

