#include "stdafx.h"
#include "dx11wrapper.h"
#include "foundation/utilities/tools.h"

#if GM_USE_DX11
#include <gmdx11/gmdx11factory.h>
#endif

typedef IFactory* (*CREATE_PROC)();
typedef void(*SHADER_LOAD_PROC)(const IRenderContext*, const GMString&);
typedef void (*SHADER_EXT_LOAD_PROC)(const IRenderContext*);

static HMODULE s_hDx11;
static CREATE_PROC s_createProc;
static SHADER_LOAD_PROC s_shaderProc;
static SHADER_EXT_LOAD_PROC s_extensionShaderProc;

extern "C"
{
	bool CreateDirectX11Factory(IFactory** ppFactory)
	{
#if GM_WINDOWS
#	if GM_USE_DX11
		if (ppFactory)
			*ppFactory = new GMDx11Factory();
		return true;
#	else
		{
			s_hDx11 = LoadLibrary(L"gamemachinedx11.dll");
			// dll无法加载，依赖模块不存在，说明没有DirectX11环境
			if (!s_hDx11)
				return false;

			if (!s_createProc)
			{
				s_createProc = (CREATE_PROC)GetProcAddress(s_hDx11, "gmdx11_CreateDirectX11Factory");
				if (!s_createProc)
				{
					FreeLibrary(s_hDx11);
					s_hDx11 = NULL;
					return false;
				}
			}
		}

		GM_ASSERT(s_createProc);
		if (ppFactory)
			*ppFactory = s_createProc();

		return true;
#	endif
#else
		return false;
#endif
	}

	void DirectX11LoadShader(const IRenderContext* context, const GMString& path)
	{
		if (!s_shaderProc)
		{
			s_shaderProc = (SHADER_LOAD_PROC) GetProcAddress(s_hDx11, "gmdx11_loadShader");
			if (!s_shaderProc)
			{
				gm_error(gm_dbg_wrap("Method not found."));
				return;
			}
			return s_shaderProc(context, path);
		}
		gm_error(gm_dbg_wrap("Invoke method failed. This method shouldn't be invoked."));
	}

	void DirectX11LoadExtensionShaders(const IRenderContext* context)
	{
		if (!s_extensionShaderProc)
		{
			s_extensionShaderProc = (SHADER_EXT_LOAD_PROC)GetProcAddress(s_hDx11, "gmdx11_loadExtensionShaders");
			if (!s_extensionShaderProc)
			{
				gm_error(gm_dbg_wrap("Method not found."));
				return;
			}
			return s_extensionShaderProc(context);
		}
		gm_error(gm_dbg_wrap("Invoke method failed. This method shouldn't be invoked."));
	}
}
