#ifndef __GM_WRAPPER_H__
#define __GM_WRAPPER_H__
#include <gmcommon.h>
#include <dx11wrapper.h>
BEGIN_NS

GMRenderEnvironment GMCreateFactory(GMRenderEnvironment prefer, GMRenderEnvironment fallback, OUT IFactory** ppFactory)
{
	if (!ppFactory)
		return GMRenderEnvironment::Invalid;

	if (prefer == gm::GMRenderEnvironment::OpenGL)
	{
		*ppFactory = new gm::GMGLFactory();
		return prefer;
	}
	else
	{
		// 希望创建一个DirectX11 (prefer == gm::GMCapability::SupportDirectX11)
#if GM_USE_DLL // 使用GameMachine.dll才能创建DirectX
		if (gm::GMQueryCapability(gm::GMCapability::SupportDirectX11))
		{
			bool bCreated = CreateDirectX11Factory(ppFactory);
			GM_ASSERT(bCreated);
			return gm::GMRenderEnvironment::DirectX11;
		}
		else
#endif
		{
			if (fallback == GMRenderEnvironment::OpenGL)
			{
				*ppFactory = new gm::GMGLFactory();
				return gm::GMRenderEnvironment::OpenGL;
			}
			return GMRenderEnvironment::Invalid;
		}
	}
}

END_NS
#endif