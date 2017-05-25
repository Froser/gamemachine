#include "stdafx.h"
#include "gmgl_renders_glyph.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmglfunc.h"

void GMGLRenders_Glyph::updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt)
{
	D(d);
	GMGL::cameraPosition(lookAt, *d->gmglShaders, GMSHADER_VIEW_POSITION);
}

void GMGLRenders_Glyph::beginShader(Shader& shader)
{
	D(d);
	d->shader = &shader;

	// 纹理
	activateShader(&shader);

	// 只选择环境光纹理
	TextureFrames& textures = shader.texture.textures[TEXTURE_INDEX_AMBIENT];

	// 获取序列中的这一帧
	ITexture* texture = getTexture(textures);
	if (texture)
	{
		// 激活动画序列
		activeTexture(d->shader, TEXTURE_INDEX_AMBIENT);
		texture->drawTexture(&textures);
	}
}