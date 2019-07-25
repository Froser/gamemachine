#ifndef __GMDX11TEXTURE_P_H__
#define __GMDX11TEXTURE_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMDx11Texture)
{
	const IRenderContext* context = nullptr;
	GMImage* image = nullptr;
	GMComPtr<ID3D11Device> device;
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMComPtr<ID3D11Resource> resource;
	GMComPtr<ID3D11ShaderResourceView> shaderResourceView;
	GMComPtr<ID3D11SamplerState> samplerState;
	GMComPtr<ID3DX11Effect> effect;
	Vector<ID3DX11EffectShaderResourceVariable*> shaderResourceVariables;
	Vector<ID3DX11EffectSamplerVariable*> samplerVariables;
};

END_NS
#endif