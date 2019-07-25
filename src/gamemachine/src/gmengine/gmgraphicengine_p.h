#ifndef __GMGRAPHICENGINE_P_H__
#define __GMGRAPHICENGINE_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_ALIGNED(GMGraphicEngine)
{
	GMThreadId mtid = 0;
	GMint32 begun = 0;
	const IRenderContext* context = nullptr;
	GMCamera camera;
	GMGlyphManager* glyphManager = nullptr;
	IFramebuffers* defaultFramebuffers = nullptr;
	IFramebuffers* filterFramebuffers = nullptr;
	GMGameObject* filterQuad = nullptr;
	GMFramebuffersStack framebufferStack;
	IGBuffer* gBuffer = nullptr;
	GMRenderConfig renderConfig;
	GMDebugConfig debugConfig;
	GMStencilOptions stencilOptions;
	Vector<ILight*> lights;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
	GMGlobalBlendStateDesc blendState;
	GMOwnedPtr<GMRenderTechniqueManager> renderTechniqueManager;
	GMOwnedPtr<GMPrimitiveManager> primitiveManager;
	GMConfigs configs;

	// Shadow
	GMShadowSourceDesc shadow;
	GMShadowSourceDesc lastShadow;
	IFramebuffers* shadowDepthFramebuffers = nullptr;
	GMMat4 shadowCameraVPmatrices[GMMaxCascades];
	bool isDrawingShadow = false;
};

END_NS
#endif