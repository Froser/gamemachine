#ifndef __GMDX11FXC_H__
#define __GMDX11FXC_H__
#include <gmcommon.h>
BEGIN_NS

#if !GM_WINDOWS
#error This file is only used in Windows.
#endif

enum class GMDx11FXCProfile
{
	FX_5_0,
	CS_5_0,
};

struct GMDx11FXCDescription
{
	GMString fxcOutputDir;
	GMString fxcOutputFilename;
	GMString code;
	GMString codePath;
	GMString entryPoint;
	GMint32 optimizationLevel = 1; // from 0-3
	bool treatWarningsAsErrors = true;
	bool debug = false;
	GMDx11FXCProfile profile = GMDx11FXCProfile::FX_5_0;
	GMBuffer sourceMd5Hint;
};

class GMDx11FXC
{
public:
	bool canLoad(const GMDx11FXCDescription& desc, const GMBuffer& fxcBuffer);
	bool load(const GMBuffer& shaderBuffer, ID3D11Device* pDevice, ID3DX11Effect** ppEffect);
	bool load(const GMBuffer& shaderBuffer, ID3D11Device* pDevice, ID3D11ComputeShader** ppComputeShader);
	bool tryLoadCache(IN OUT GMDx11FXCDescription& desc, ID3D11Device* pDevice, ID3DX11Effect** ppEffect);
	bool tryLoadCache(IN OUT GMDx11FXCDescription& desc, ID3D11Device* pDevice, ID3D11ComputeShader** ppComputeShader);
	bool compile(IN OUT GMDx11FXCDescription&, ID3DBlob** ppCode, ID3DBlob** ppErrorMessages);
};

END_NS
#endif