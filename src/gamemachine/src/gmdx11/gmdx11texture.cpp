#include "stdafx.h"
#include "gmdx11texture.h"
#include <gamemachine.h>

namespace
{
	inline D3D11_SRV_DIMENSION toDxSrvDimension(GMTextureTarget target)
	{
		switch (target)
		{
		case GMTextureTarget::Texture1D:
			return D3D_SRV_DIMENSION_TEXTURE1D;
		case GMTextureTarget::Texture1DArray:
			return D3D_SRV_DIMENSION_TEXTURE1DARRAY;
		case GMTextureTarget::Texture2D:
			return D3D_SRV_DIMENSION_TEXTURE2D;
		case GMTextureTarget::Texture2DArray:
			return D3D_SRV_DIMENSION_TEXTURE2DARRAY;
		case GMTextureTarget::Texture3D:
			return D3D_SRV_DIMENSION_TEXTURE3D;
		case GMTextureTarget::CubeMap:
			return D3D_SRV_DIMENSION_TEXTURECUBE;
		case GMTextureTarget::CubeMapArray:
			return D3D_SRV_DIMENSION_TEXTURECUBEARRAY;
		default:
			GM_ASSERT(false);
			return D3D_SRV_DIMENSION_UNKNOWN;
		}
	}
}

GMDx11Texture::GMDx11Texture(GMImage* image)
{
	D(d);
	d->image = image;
	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&d->device);
	GM_ASSERT(d->device);

	init();
}

void GMDx11Texture::drawTexture(GMTextureFrames* frames)
{
}

void GMDx11Texture::init()
{
	D(d);
	auto& imageData = d->image->getData();
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;

	desc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
	desc.ViewDimension = toDxSrvDimension(imageData.target);

	GM_DX_HR(d->device->CreateShaderResourceView(
		NULL,
		&desc,
		&d->shaderResourceView
	));
}