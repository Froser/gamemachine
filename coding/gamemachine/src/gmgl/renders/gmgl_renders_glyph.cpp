#include "stdafx.h"
#include "gmgl_renders_glyph.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmglshaderprogram.h"

void GMGLRenders_Glyph::updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt)
{
	D(d);
	GMfloat vec[4] = { lookAt.position[0], lookAt.position[1], lookAt.position[2], 1.0f };
	d->gmglShaderProgram->setMatrix4(GMSHADER_VIEW_POSITION, vec);
}

void GMGLRenders_Glyph::beginShader(Shader& shader, GMDrawMode mode)
{
	D(d);
	d->shader = &shader;

	// 应用Shader
	activateShader();

	// 只选择环境光纹理
	GMTextureFrames& textures = shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, 0);

	// 获取序列中的这一帧
	ITexture* texture = getTexture(textures);
	if (texture)
	{
		// 激活动画序列
		activateTexture(GMTextureType::AMBIENT, 0);
		texture->drawTexture(&textures);
	}
}