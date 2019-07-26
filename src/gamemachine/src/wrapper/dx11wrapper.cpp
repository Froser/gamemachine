#include "stdafx.h"
#include "dx11wrapper.h"
#include "foundation/utilities/tools.h"

#if GM_USE_DX11
#include <gmdx11/gmdx11factory.h>
#include "gmdx11/main.h"
#endif

BEGIN_NS

#if GM_WINDOWS
typedef IFactory* (*CREATE_PROC)();
typedef void(*SHADER_LOAD_PROC)(const IRenderContext*, const GMString&);
typedef void (*SHADER_EXT_LOAD_PROC)(const IRenderContext*);
typedef void(*EXT_WAVE_OBJECT_SHADER_RENDER_PROC)(const gm::GMWaveGameObject* waveObject, IShaderProgram* shaderProgram);

static HMODULE s_hDx11;
static CREATE_PROC s_createProc;
static SHADER_LOAD_PROC s_shaderProc;
static SHADER_EXT_LOAD_PROC s_extensionShaderProc;
static EXT_WAVE_OBJECT_SHADER_RENDER_PROC s_waveObjShaderRenderProc;
#endif

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
				s_createProc = (CREATE_PROC)GetProcAddress(s_hDx11, "gmdx11_createDirectX11Factory");
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
#if GM_WINDOWS
#if GM_USE_DX11
		gmdx11_loadShader(context, path);
#else
		if (!s_shaderProc)
		{
			s_shaderProc = (SHADER_LOAD_PROC) GetProcAddress(s_hDx11, "gmdx11_loadShader");
			if (!s_shaderProc)
			{
				gm_error(gm_dbg_wrap("Method not found."));
				return;
			}
		}
		if (s_shaderProc)
			return s_shaderProc(context, path);
		gm_error(gm_dbg_wrap("Invoke method failed. This method shouldn't be invoked."));
#endif
#else
		GM_ASSERT(false); //shouldn't be here
#endif
	}

	void DirectX11LoadExtensionShaders(const IRenderContext* context)
	{
#if GM_WINDOWS
#if GM_USE_DX11
		gmdx11_loadExtensionShaders(context);
#else
		if (!s_extensionShaderProc)
		{
			s_extensionShaderProc = (SHADER_EXT_LOAD_PROC)GetProcAddress(s_hDx11, "gmdx11_loadExtensionShaders");
			if (!s_extensionShaderProc)
			{
				gm_error(gm_dbg_wrap("Method not found."));
				return;
			}
		}
		if (s_extensionShaderProc)
			return s_extensionShaderProc(context);
		gm_error(gm_dbg_wrap("Invoke method failed. This method shouldn't be invoked."));
#endif
#else
		GM_ASSERT(false); //shouldn't be here
#endif
	}

	void Ext_RenderWaveObjectShader(gm::GMWaveGameObject* waveObject, IShaderProgram* shaderProgram)
	{
#if GM_WINDOWS
#if GM_USE_DX11
		gmdx11_ext_renderWaveObjectShader(waveObject, shaderProgram);
#else
		if (!s_waveObjShaderRenderProc)
		{
			s_waveObjShaderRenderProc = (EXT_WAVE_OBJECT_SHADER_RENDER_PROC)GetProcAddress(s_hDx11, "gmdx11_ext_renderWaveObjectShader");
			if (!s_waveObjShaderRenderProc)
			{
				gm_error(gm_dbg_wrap("Method not found."));
				return;
			}
		}
		if (s_waveObjShaderRenderProc)
			return s_waveObjShaderRenderProc(waveObject, shaderProgram);
		gm_error(gm_dbg_wrap("Invoke method failed. This method shouldn't be invoked."));
#endif
#else
		GM_ASSERT(false); //shouldn't be here
#endif
	}
}

END_NS