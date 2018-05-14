#include "stdafx.h"
#include <GL/glew.h>
#include "gmglglyphmanager.h"
#include "gmgltexture.h"
#include "freetype/ftglyph.h"

BEGIN_NS
class GMGLGlyphTexture : public ITexture
{
public:
	GMGLGlyphTexture() = default;
	~GMGLGlyphTexture()
	{
		glDeleteTextures(1, &m_id);
	}

public:
	virtual void init() override
	{
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexStorage2D(GL_TEXTURE_2D,
			1,
			GL_R8,
			GMGLGlyphManager::CANVAS_WIDTH,
			GMGLGlyphManager::CANVAS_HEIGHT
		);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	virtual void bindSampler(GMTextureSampler* sampler) override
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	virtual void useTexture(GMint textureIndex) override
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_id);
	}

	GMuint getTextureId()
	{
		return m_id;
	}

private:
	GMuint m_id;
};
END_NS

GMGLGlyphManager::GMGLGlyphManager()
{
	D(d);
	d->cursor_u = d->cursor_v = 0;
	d->maxHeight = 0;
	d->texture = new GMGLGlyphTexture();
	d->texture->init();
}

GMGLGlyphManager::~GMGLGlyphManager()
{
	D(d);
	GM_delete(d->texture);
}

ITexture* GMGLGlyphManager::glyphTexture()
{
	D(d);
	return d->texture;
}

void GMGLGlyphManager::updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo)
{
	D(d);
	// 创建纹理
	glBindTexture(GL_TEXTURE_2D, d->texture->getTextureId());
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 使用一个字节保存，必须设置对齐为1
	glTexSubImage2D(GL_TEXTURE_2D,
		0,
		glyphInfo.x,
		glyphInfo.y,
		glyphInfo.width,
		glyphInfo.height,
		GL_RED,
		GL_UNSIGNED_BYTE,
		bitmapGlyph.buffer);

	glBindTexture(GL_TEXTURE_2D, 0);
}