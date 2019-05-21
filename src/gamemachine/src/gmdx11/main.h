#ifndef __GMDX11_MAIN_H__
#define __GMDX11_MAIN_H__
#include <gmcommon.h>

extern "C"
{
	GM_DECL_EXPORT IFactory* gmdx11_CreateDirectX11Factory();
	GM_DECL_EXPORT void gmdx11_loadShader(const IRenderContext*, const GMString& fileName);
	GM_DECL_EXPORT void gmdx11_loadExtensionShaders(const IRenderContext*);
}

#endif