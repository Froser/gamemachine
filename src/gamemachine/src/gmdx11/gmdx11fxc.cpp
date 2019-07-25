#include "stdafx.h"
#include "gmdx11fxc.h"
#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <strsafe.h>
#include <fstream>
#include <gmcom.h>
#include <foundation/gmcryptographic.h>

#define TEMP_CODE_FILE "code.hlsl"
#define LOG_FILE "log.txt"
#define BUFSIZE 10 * 1024

#define GM_HEADER "GameMachine_"
#define GM_HEADER_LEN 12
#define GM_FXC_VERSION_MAJOR 1
#define GM_FXC_VERSION_MINOR 0

BEGIN_NS

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
			return buf.getData();
		}

		virtual SIZE_T STDMETHODCALLTYPE GetBufferSize(void)
		{
			return buf.getSize();
		}

		STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv)
		{
			if (IsEqualIID(iid, IID_IUnknown))
			{
				*ppv = (IUnknown *) this;
			}

			return E_NOINTERFACE;
		}

		STDMETHOD_(ULONG, AddRef)()
		{
			return ++refCount;
		}

		STDMETHOD_(ULONG, Release)()
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

	public:
		template <typename T>
		static void createBlob(T&& buffer, OUT ID3DBlob** ppBlob)
		{
			if (ppBlob)
				*ppBlob = new GMBlob(std::forward<T>(buffer));
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
		GM_ASSERT(fxcBuffer.getData());

		GMsize_t uOffset = 0;
		if (memcmp(fxcBuffer.getData(), GM_HEADER, GM_HEADER_LEN) != 0)
			return false;

		uOffset += GM_HEADER_LEN;
		memcpy_s(&version, sizeof(Version), fxcBuffer.getData() + uOffset, sizeof(Version));

		uOffset += sizeof(Version);
		memcpy_s(fingerprints, 16, fxcBuffer.getData() + uOffset, 16);

		contentPtr = uOffset + 16;
		return true;
	}

	void sourceCodeToBufferW(const GMString& code, REF GMBuffer& buffer)
	{
		buffer = GMBuffer::createBufferView((GMbyte*)code.c_str(), code.length() * sizeof(GMwchar));
	}

	void sourceCodeToBufferA(const std::string& code, REF GMBuffer& buffer)
	{
		buffer = GMBuffer::createBufferView((GMbyte*)code.c_str(), code.length());
	}

	bool readGMFXCFile(const GMDx11FXCDescription& desc, GMBuffer& buf)
	{
		GM_ASSERT(!desc.fxcOutputFilename.isEmpty());
		if (desc.fxcOutputFilename.isEmpty())
		{
			gm_warning(gm_dbg_wrap("FXC output filename must be specified."));
			return false;
		}

		GMString outputPath = desc.fxcOutputDir + L"\\" + desc.fxcOutputFilename;
		HANDLE hFile = CreateFile(outputPath.c_str(), GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		DWORD dwSize = GetFileSize(hFile, NULL);
		buf.resize(dwSize);
		ReadFile(hFile, buf.getData(), (DWORD)buf.getSize(), NULL, NULL);
		CloseHandle(hFile);
		return true;
	}

	bool fillDescription(GMDx11FXCDescription* desc)
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

	bool makeFingerprints(const GMDx11FXCDescription& desc, ID3DBlob* pCode)
	{
		TCHAR fxcOutput[MAX_PATH];
		PathCombine(fxcOutput, desc.fxcOutputDir.c_str(), desc.fxcOutputFilename.toStdWString().c_str());

		// 再次打开生成文件，创建带有GM标识头的fxc文件，在文件头加上GameMachine {版本号(int32)}{代码MD5}。
		HANDLE hFile = CreateFile(fxcOutput, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		GMBuffer sourceCode;
		sourceCodeToBufferW(desc.code, sourceCode);

		GMBuffer md5 = desc.sourceMd5Hint;
		if (md5.getSize() != 16)
			GMCryptographic::hash(sourceCode, GMCryptographic::MD5, md5);

		// 准备一块空间content，这段缓存就是最终文件的内容
		GM_ASSERT(md5.getSize() == 16);
		constexpr GMsize_t HEADER_LEN = GM_HEADER_LEN + sizeof(Version) + 16; //16表示16字节的MD5
		GMBuffer header;
		header.resize(HEADER_LEN);

		// 写 "GameMachine"
		GMsize_t uOffset = GM_HEADER_LEN;
		memcpy_s(header.getData(), header.getSize(), GM_HEADER, uOffset);

		// 写 版本号 Major Minor
		Version version = {
			GM_FXC_VERSION_MAJOR,
			GM_FXC_VERSION_MINOR,
		};
		memcpy_s(header.getData() + uOffset, header.getSize() - uOffset, &version, sizeof(Version));
		uOffset += sizeof(Version);

		// 写MD5
		memcpy_s(header.getData() + uOffset, header.getSize() - uOffset, md5.getData(), md5.getSize());
		uOffset += md5.getSize();

		// 获取编译好的二进制代码
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		if (!WriteFile(hFile, header.getData(), (DWORD)header.getSize(), NULL, NULL))
			return false;

		if (!WriteFile(hFile, pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, NULL))
			return false;

		SetEndOfFile(hFile);
		CloseHandle(hFile);
		return true;
	}
}

bool GMDx11FXC::canLoad(const GMDx11FXCDescription& desc, const GMBuffer& fxcBuffer)
{
	if (!fxcBuffer.getData())
		return false;

	// 检查Header
	Version version;
	GMbyte fileFingerprints[16];
	GMsize_t contentOffset = 0;
	if (!getFileHeaders(fxcBuffer, version, fileFingerprints, contentOffset))
	{
		gm_warning(gm_dbg_wrap("GXC wrong header."));
		return false;
	}

	GMBuffer md5 = desc.sourceMd5Hint;
	if (md5.getSize() != 16)
	{
		GMBuffer sourceCode;
		sourceCodeToBufferW(desc.code, sourceCode);
		GMCryptographic::hash(sourceCode, GMCryptographic::MD5, md5);
		GM_ASSERT(md5.getSize() == 16);
	}

	if (memcmp(fileFingerprints, md5.getData(), md5.getSize()) != 0)
	{
		gm_warning(gm_dbg_wrap("Shader code doesn't match gxc file."));
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

	const char* profile = nullptr;
	switch (desc.profile)
	{
	case GMDx11FXCProfile::CS_5_0:
		profile = "cs_5_0";
		break;
	case GMDx11FXCProfile::FX_5_0:
	default:
		profile = "fx_5_0";
		break;
	}
	GM_ASSERT(profile);

	gm_info(gm_dbg_wrap("Compiling HLSL code..."));
	std::string strCodePath = desc.codePath.toStdString();
	std::string entryPoint = desc.entryPoint.toStdString();
	if (SUCCEEDED(D3DCompile(
		bufCode.getData(),
		bufCode.getSize(),
		strCodePath.c_str(),
		NULL,
		0,
		entryPoint.c_str(),
		profile,
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
		const GMBuffer gfxView = GMBuffer::createBufferView(shaderBuffer, uOffset);
		GMComPtr<ID3DBlob> shaderBufferBlob;
		GMBlob::createBlob(gfxView, &shaderBufferBlob);
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

bool GMDx11FXC::load(const GMBuffer& shaderBuffer, ID3D11Device* pDevice, ID3D11ComputeShader** ppComputeShader)
{
	Version version;
	GMbyte fingerprints[16];
	GMsize_t uOffset = 0;
	getFileHeaders(shaderBuffer, version, fingerprints, uOffset);
	if (version.major == 1 && version.minor == 0)
	{
		const GMBuffer gfxView = GMBuffer::createBufferView(shaderBuffer, uOffset);
		GMComPtr<ID3DBlob> shaderBufferBlob;
		GMBlob::createBlob(gfxView, &shaderBufferBlob);
		return SUCCEEDED(pDevice->CreateComputeShader(shaderBufferBlob->GetBufferPointer(), shaderBufferBlob->GetBufferSize(), nullptr, ppComputeShader));
	}

	gm_warning(gm_dbg_wrap("The version {0}.{1} is not supported."), GMString(version.major), GMString(version.minor));
	return false;
}

bool GMDx11FXC::tryLoadCache(GMDx11FXCDescription& desc, ID3D11Device* pDevice, ID3DX11Effect** ppEffect)
{
	if (!fillDescription(&desc))
		return false;

	GMBuffer buf;
	if (!readGMFXCFile(desc, buf))
		return false;

	return (canLoad(desc, buf) && load(buf, pDevice, ppEffect));
}

bool GMDx11FXC::tryLoadCache(IN OUT GMDx11FXCDescription& desc, ID3D11Device* pDevice, ID3D11ComputeShader** ppComputeShader)
{
	if (!fillDescription(&desc))
		return false;

	GMBuffer buf;
	if (!readGMFXCFile(desc, buf))
		return false;

	return (canLoad(desc, buf) && load(buf, pDevice, ppComputeShader));
}

END_NS