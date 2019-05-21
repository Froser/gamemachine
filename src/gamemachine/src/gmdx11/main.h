#ifndef __GMDX11_MAIN_H__
#define __GMDX11_MAIN_H__
#include <gmcommon.h>
#include "extensions/objects/gmwavegameobject.h"

extern "C"
{
	GM_DECL_EXPORT IFactory* gmdx11_createDirectX11Factory();
	GM_DECL_EXPORT void gmdx11_loadShader(const IRenderContext*, const GMString& fileName);
	GM_DECL_EXPORT void gmdx11_loadExtensionShaders(const IRenderContext*);
	GM_DECL_EXPORT void gmdx11_ext_renderWaveObjectShader(const GMWaveGameObject* waveObject, IShaderProgram* shaderProgram);
}

#endif