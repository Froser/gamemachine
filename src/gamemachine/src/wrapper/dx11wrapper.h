#ifndef __WRAPPER_DX11WRAPPER_H__
#define __WRAPPER_DX11WRAPPER_H__
#include <gmcommon.h>

struct ID3DX11Effect;

namespace gm
{
	class GMWaveGameObject;
	struct ITechnique;
}

extern "C"
{
	GM_EXPORT bool CreateDirectX11Factory(gm::IFactory** ppFactory);
	GM_EXPORT void DirectX11LoadShader(const gm::IRenderContext*, const gm::GMString&);
	GM_EXPORT void DirectX11LoadExtensionShaders(const gm::IRenderContext*);
	GM_EXPORT void Ext_RenderWaveObjectShader(gm::GMWaveGameObject* waveObject, gm::IShaderProgram* shaderProgram);
	GM_EXPORT ID3DX11Effect* DirectX11GetEffectFromTechnique(gm::ITechnique* technique);
};

#endif