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
		"deferred_light_pass_gPosition",
		"deferred_light_pass_gNormal",
		"deferred_light_pass_gNormal_eye",
		"deferred_light_pass_gTexAmbient",
		"deferred_light_pass_gTexDiffuse",
		"deferred_light_pass_gTangent_eye",
		"deferred_light_pass_gBitangent_eye",
		"deferred_light_pass_gNormalMap",
	};

	Array<std::string, 2> s_GBufferMaterialUniformNames =
	{
		"deferred_material_pass_gKs_gShininess", //gKs: 3, gShininess: 1
		"deferred_material_pass_gHasNormalMap_gRefractivity", //gHasNormalMap: 1, gRefractivity: 1
	};
}

/*
void GMGLGBufferDep::copyDepthBuffer(GLuint target)
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo[(GMint)GMGLDeferredRenderState::PassingGeometry]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
	glBlitFramebuffer(0, 0, d->renderWidth, d->renderHeight, 0, 0, d->renderWidth, d->renderHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
*/

GMGLGBuffer::GMGLGBuffer(GMGLGraphicEngine* engine)
{
	D(d);
	d->engine = engine;
}

IFramebuffers* GMGLGBuffer::createGeometryFramebuffers()
{
	IFramebuffers* framebuffers = nullptr;
	const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
	GMFramebufferDesc desc = { 0 };
	desc.rect = states.renderRect;

	// Geometry Pass的纹理格式为R8G8B8A8_UNORM，意味着所有的输出在着色器中范围是[0,1]，对应着UNORM的[0x00, 0xFF]
	// 对于[-1, 1]范围的数据，需要在着色器中进行一次转换。
	desc.framebufferFormat = GMFramebufferFormat::R8G8B8A8_UNORM;

	GM.getFactory()->createFramebuffers(&framebuffers);
	framebuffers->init(desc);
	GM_ASSERT(framebuffers);

	constexpr GMuint framebufferCount = GM_array_size(s_GBufferGeometryUniformNames);
	GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
	for (GMint i = 0; i < framebufferCount; ++i)
	{
		IFramebuffer* framebuffer = nullptr;
		GM.getFactory()->createFramebuffer(&framebuffer);
		GM_ASSERT(framebuffer);
		framebuffer->init(desc);
		framebuffers->addFramebuffer(framebuffer);
	}
	return framebuffers;
}

IFramebuffers* GMGLGBuffer::createMaterialFramebuffers()
{
	IFramebuffers* framebuffers = nullptr;
	const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
	GMFramebufferDesc desc = { 0 };
	desc.rect = states.renderRect;
	desc.framebufferFormat = GMFramebufferFormat::R32G32B32A32_FLOAT;

	GM.getFactory()->createFramebuffers(&framebuffers);
	framebuffers->init(desc);
	GM_ASSERT(framebuffers);

	constexpr GMuint framebufferCount = GM_array_size(s_GBufferMaterialUniformNames);
	GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
	for (GMint i = 0; i < framebufferCount; ++i)
	{
		IFramebuffer* framebuffer = nullptr;
		GM.getFactory()->createFramebuffer(&framebuffer);
		GM_ASSERT(framebuffer);
		framebuffer->init(desc);
		framebuffers->addFramebuffer(framebuffer);
	}
	return framebuffers;
}

void GMGLGBuffer::geometryPass(GMGameObject *objects[], GMuint count)
{
	D(d);
	IFramebuffers* activeFramebuffers = nullptr;

	setGeometryPassingState(GMGeometryPassingState::PassingGeometry);
	activeFramebuffers = getGeometryFramebuffers();
	activeFramebuffers->clear();
	activeFramebuffers->bind();
	d->engine->draw(objects, count);
	activeFramebuffers->unbind();

	setGeometryPassingState(GMGeometryPassingState::PassingMaterial);
	activeFramebuffers = getMaterialFramebuffers();
	activeFramebuffers->clear();
	activeFramebuffers->bind();
	d->engine->draw(objects, count);
	activeFramebuffers->unbind();

	setGeometryPassingState(GMGeometryPassingState::Done);
}

void GMGLGBuffer::lightPass()
{
	D(d);
	GM_ASSERT(getQuad());
	getQuad()->draw();
}

void GMGLGBuffer::drawGeometryBuffer(GMuint index, const GMRect& rect)
{
	D(d);
	GMGLFramebuffers* source = gm_cast<GMGLFramebuffers*>(getGeometryFramebuffers());
	glDisable(GL_DEPTH_TEST);
	GLint dest = 0;
	const GMFramebufferDesc& desc = source->getDesc();

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &dest);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, source->framebufferId());
	glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
	glBlitFramebuffer(
		0, 0, desc.rect.width, desc.rect.height,
		rect.x, rect.y, rect.width, rect.height,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, dest);
}

const std::string* GMGLGBuffer::GBufferGeometryUniformNames()
{
	return s_GBufferGeometryUniformNames.data();
}

const std::string* GMGLGBuffer::GBufferMaterialUniformNames()
{
	return s_GBufferMaterialUniformNames.data();
}