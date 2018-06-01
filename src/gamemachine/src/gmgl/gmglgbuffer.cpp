#include "stdafx.h"
#include <GL/glew.h>
#include "gmglgbuffer.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"
#include "shader_constants.h"
#include "gmgltexture.h"
#include "gmglframebuffer.h"

#define GMEngine static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine())

namespace
{
	Array<std::string, 8> s_GBufferGeometryUniformNames =
	{
		"deferred_light_pass_gPosition_Refractivity",
		"deferred_light_pass_gNormal_IlluminationModel",
		"deferred_light_pass_gTexAmbientAlbedo",
		"deferred_light_pass_gTexDiffuseMetallicRoughnessAO",
		"deferred_light_pass_gTangent_eye",
		"deferred_light_pass_gBitangent_eye",
		"deferred_light_pass_gNormalMap_bNormalMap",
		"deferred_light_pass_gKs_Shininess_F0",
	};
}

GMGLGBuffer::GMGLGBuffer(const GMContext* context)
	: GMGBuffer(context)
{
	D(d);
	d->engine = gm_cast<GMGraphicEngine*>(context->engine);
}

IFramebuffers* GMGLGBuffer::createGeometryFramebuffers()
{
	D(d);
	IFramebuffers* framebuffers = nullptr;
	const GMWindowStates& windowStates = d->context->window->getWindowStates();
	GMFramebufferDesc desc = { 0 };
	desc.rect = windowStates.renderRect;

	// Geometry Pass的纹理格式为R8G8B8A8_UNORM，意味着所有的输出在着色器中范围是[0,1]，对应着UNORM的[0x00, 0xFF]
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
	GM_ASSERT(framebuffers);

	constexpr GMuint framebufferCount = GM_array_size(s_GBufferGeometryUniformNames);
	GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
	for (GMint i = 0; i < framebufferCount; ++i)
	{
		IFramebuffer* framebuffer = nullptr;
		GM.getFactory()->createFramebuffer(d->context, &framebuffer);
		GM_ASSERT(framebuffer);
		desc.framebufferFormat = formats[i];
		framebuffer->init(desc);
		framebuffers->addFramebuffer(framebuffer);
	}
	return framebuffers;
}

void GMGLGBuffer::geometryPass(const List<GMGameObject*>& objects)
{
	D(d);
	IFramebuffers* activeFramebuffers = nullptr;

	setGeometryPassingState(GMGeometryPassingState::PassingGeometry);
	activeFramebuffers = getGeometryFramebuffers();
	activeFramebuffers->clear();
	activeFramebuffers->bind();
	d->engine->draw(objects);
	activeFramebuffers->unbind();
	setGeometryPassingState(GMGeometryPassingState::Done);
}

void GMGLGBuffer::lightPass()
{
	D(d);
	GM_ASSERT(getQuad());
	getQuad()->draw(d->context);
}

void GMGLGBuffer::drawGeometryBuffer(GMuint index, const GMRect& rect)
{
	D(d);
	GMGLFramebuffers* source = gm_cast<GMGLFramebuffers*>(getGeometryFramebuffers());
	GLint dest = 0;
	const GMFramebufferDesc& desc = source->getDesc();

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &dest);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, source->framebufferId());
	glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
	glBlitFramebuffer(
		0, 0, desc.rect.width, desc.rect.height,
		rect.x, rect.y, rect.width, rect.height,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, dest);
}

const std::string* GMGLGBuffer::GBufferGeometryUniformNames()
{
	return s_GBufferGeometryUniformNames.data();
}