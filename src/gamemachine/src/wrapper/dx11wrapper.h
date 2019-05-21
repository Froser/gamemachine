#ifndef __WRAPPER_DX11WRAPPER_H__
#define __WRAPPER_DX11WRAPPER_H__
#include <gmcommon.h>

namespace gm
{
	class GMWaveGameObject;
}

extern "C"
{
	GM_EXPORT bool CreateDirectX11Factory(gm::IFactory** ppFactory);
	GM_EXPORT void DirectX11LoadShader(const gm::IRenderContext*, const gm::GMString&);
	GM_EXPORT void DirectX11LoadExtensionShaders(const gm::IRenderContext*);
	GM_EXPORT void Ext_RenderWaveObjectShader(const gm::GMWaveGameObject* waveObject, gm::IShaderProgram* shaderProgram);
};

#endif