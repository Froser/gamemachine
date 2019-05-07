#ifndef __GMDX11FXC_H__
#define __GMDX11FXC_H__
#include <gmcommon.h>
BEGIN_NS

#if !GM_WINDOWS
#error This file is only used in Windows.
#endif

struct GMDx11FXCDescription
{
	GMString fxcOutputDir;
	GMString code;
	GMString codePath;
	GMint32 optimizationLevel; // from 0-3
	bool treatWarningsAsErrors;
	bool debug;
};

class GMDx11FXC
{
public:
	bool canLoad(const GMString& code, const GMBuffer& fxcBuffer);
	bool load(const GMBuffer& shaderBuffer, ID3D11Device* pDevice, ID3DX11Effect** ppEffect);
	bool tryLoadCache(IN OUT GMDx11FXCDescription& desc, ID3D11Device* pDevice, ID3DX11Effect** ppEffect);
	bool compile(IN OUT GMDx11FXCDescription&, ID3DBlob** ppCode, ID3DBlob** ppErrorMessages);

private:
	bool fillDescription(GMDx11FXCDescription*);
	bool makeFingerprints(const GMDx11FXCDescription& desc, ID3DBlob* pCode);
};

END_NS
#endif