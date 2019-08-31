#ifndef __GMDX11_MAIN_H__
#define __GMDX11_MAIN_H__
#include <gmcommon.h>

struct ID3DX11Effect;

BEGIN_NS
class GMWaveGameObject;
END_NS

extern "C"
{
	GM_DECL_EXPORT gm::IFactory* gmdx11_createDirectX11Factory();
	GM_DECL_EXPORT void gmdx11_loadShader(const gm::IRenderContext*, const gm::GMString& fileName);
	GM_DECL_EXPORT void gmdx11_loadExtensionShaders(const gm::IRenderContext*);
	GM_DECL_EXPORT void gmdx11_ext_renderWaveObjectShader(gm::GMWaveGameObject* waveObject, gm::IShaderProgram* shaderProgram);
	GM_DECL_EXPORT ID3DX11Effect* gmdx11_getEffectFromTechnique(gm::ITechnique* technique);
}

#endif