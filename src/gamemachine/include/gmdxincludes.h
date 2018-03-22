#ifndef __GMDXINCLUDES_H__
#define __GMDXINCLUDES_H__
#	if GM_USE_DX11
#		pragma warning(push)
#		pragma warning(disable:4005)
#		include <d3d11.h>
#		include <D3DX11async.h>
#		include <dxerr.h>
#		include "../src/gmdx11/effects/Inc/d3dx11effect.h"
#		include "../src/foundation/platforms/windows/dxmath/DirectXMath.h"
#		pragma warning(pop)
#		ifndef D3D_SET_OBJECT_NAME_N_A
#			define D3D_SET_OBJECT_NAME_N_A(pObject, Chars, pName) (pObject)->SetPrivateData(WKPDID_D3DDebugObjectName, Chars, pName)
#		endif
#		ifndef D3D_SET_OBJECT_NAME_A
#			define D3D_SET_OBJECT_NAME_A(pObject, pName) D3D_SET_OBJECT_NAME_N_A(pObject, lstrlenA(pName), pName)
#		endif
#		if _DEBUG
#			ifndef GM_DX_HR
#				define GM_DX_HR(x)											\
				{															\
					HRESULT hr = (x);										\
					if(FAILED(hr))											\
					{														\
						DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true);	\
					}														\
				}
#			endif
#		else
#			ifndef GM_DX_HR
#			define GM_DX_HR(x) (x)
#			endif
#		endif
#	endif
#endif