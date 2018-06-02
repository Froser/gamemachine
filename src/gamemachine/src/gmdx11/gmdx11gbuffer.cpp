#include "stdafx.h"
#include "gmdx11gbuffer.h"
#include "foundation/gamemachine.h"
#include "gmdx11graphic_engine.h"
#include "foundation/utilities/utilities.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmdata/gmmodel.h"
#include "gmdx11texture.h"

namespace
{
	constexpr const char* GeometryFramebufferNames[] = {
		"GM_DeferredPosition_World_Refractivity",
		"GM_DeferredNormal_World_IlluminationModel",
		"GM_DeferredTextureAmbientAlbedo",
		"GM_DeferredTextureDiffuseMetallicRoughnessAO",
		"GM_DeferredTangent_Eye",
		"GM_DeferredBitangent_Eye",
		"GM_DeferredNormalMap_bNormalMap",
		"GM_DeferredSpecular_Shininess_F0",
	};

	constexpr const char* GeometryMSAAFramebufferNames[] = {
		"GM_DeferredPosition_World_Refractivity_MSAA",
		"GM_DeferredNormal_World_IlluminationModel_MSAA",
		"GM_DeferredTextureAmbientAlbedo_MSAA",
		"GM_DeferredTextureDiffuseMetallicRoughnessAO_MSAA",
		"GM_DeferredTangent_Eye_MSAA",
		"GM_DeferredBitangent_Eye_MSAA",
		"GM_DeferredNormalMap_bNormalMap_MSAA",
		"GM_DeferredSpecular_Shininess_F0_MSAA",
	};
}

GMDx11GBuffer::GMDx11GBuffer(const IRenderContext* context)
	: GMGBuffer(context)
{
}

void GMDx11GBuffer::geometryPass(const List<GMGameObject*>& objects)
{
	D(d);
	setGeometryPassingState(GMGeometryPassingState::PassingGeometry);
	getGeometryFramebuffers()->clear();
	d->engine->draw(objects);
	setGeometryPassingState(GMGeometryPassingState::Done);
}

void GMDx11GBuffer::lightPass()
{
	D(d);
	GM_ASSERT(getQuad());
	getQuad()->draw();
}

void GMDx11GBuffer::useGeometryTextures(ID3DX11Effect* effect)
{
	D(d);
	IFramebuffers* geometryFramebuffers = getGeometryFramebuffers();
	GMsize_t cnt = geometryFramebuffers->count();
	for (GMsize_t i = 0; i < cnt; ++i)
	{
		GM_ASSERT(i < GM_array_size(GeometryFramebufferNames));
		GMDx11Texture* tex = gm_cast<GMDx11Texture*>(geometryFramebuffers->getFramebuffer(i)->getTexture());
		ID3DX11EffectShaderResourceVariable* shaderResource = effect->GetVariableByName(
			isMultisamping() ? GeometryMSAAFramebufferNames[i] : GeometryFramebufferNames[i]
		)->AsShaderResource();
		GM_ASSERT(shaderResource->IsValid());
		GM_DX_HR(shaderResource->SetResource(tex->getResourceView()));
	}
}

IFramebuffers* GMDx11GBuffer::createGeometryFramebuffers()
{
	D(d);
	IFramebuffers* framebuffers = nullptr;
	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
	GMFramebufferDesc desc = { 0 };
	desc.rect = windowStates.renderRect;

	// Geometry Pass的纹理格式为R8G8B8A8_UNORM时，意味着所有的输出在着色器中范围是[0,1]，对应着UNORM的[0x00, 0xFF]
	// 对于[-1, 1]范围的数据，需要在着色器中进行一次转换。
	GMFramebufferFormat formats[] = {
		GMFramebufferFormat::R32G32B32A32_FLOAT,
		GMFramebufferFormat::R32G32B32A32_FLOAT,
		GMFramebufferFormat::R32G32B32A32_FLOAT,
		GMFramebufferFormat::R32G32B32A32_FLOAT,
		GMFramebufferFormat::R8G8B8A8_UNORM,
		GMFramebufferFormat::R8G8B8A8_UNORM,
		GMFramebufferFormat::R8G8B8A8_UNORM,
		GMFramebufferFormat::R32G32B32A32_FLOAT,
	};

	GM.getFactory()->createFramebuffers(d->context, &framebuffers);
	GMFramebuffersDesc fbDesc;
	fbDesc.rect = windowStates.renderRect;
	framebuffers->init(fbDesc);

	constexpr GMint framebufferCount = GM_array_size(GeometryFramebufferNames); //一共有8个SV_TARGET
	GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
	for (GMint i = 0; i < framebufferCount; ++i)
	{
		IFramebuffer* framebuffer = nullptr;
		GM.getFactory()->createFramebuffer(d->context, &framebuffer);
		framebuffer->setName(GeometryFramebufferNames[i]);
		desc.framebufferFormat = formats[i];
		framebuffer->init(desc);
		framebuffers->addFramebuffer(framebuffer);
	}
	return framebuffers;
}

bool GMDx11GBuffer::isMultisamping()
{
	D(d);
	return d->context->getWindow()->getWindowStates().sampleCount > 1;
}