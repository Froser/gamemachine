#include "stdafx.h"
#include <GL/glew.h>
#include "gmglglyphmanager.h"
#include "gmgltexture.h"
#include "freetype/ftglyph.h"

namespace
{
	void flipVertically(GMbyte* data, GMuint width, GMuint height)
	{
		const GMuint bytePerPixel = 1;
		GMuint rowsToSwap = height % 2 == 1 ? (height - 1) / 2 : height / 2;
		GMbyte* tempRow = new GMbyte[width * bytePerPixel];
		for (GMuint i = 0; i < rowsToSwap; ++i)
		{
			memcpy(tempRow, &data[i * width * bytePerPixel], width * bytePerPixel);
			memcpy(&data[i * width * bytePerPixel], &data[(height - i - 1) * width * bytePerPixel], width * bytePerPixel);
			memcpy(&data[(height - i - 1) * width * bytePerPixel], tempRow, width * bytePerPixel);
		}

		GM_delete_array(tempRow);
	}
}

BEGIN_NS
class GMGLGlyphTexture : public ITexture
{
public:
	GMGLGlyphTexture()
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

	~GMGLGlyphTexture()
	{
		glDeleteTextures(1, &m_id);
	}

	virtual void drawTexture(GMTextureFrames* frames) override
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

void GMGLGlyphManager::createTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo)
{
	D(d);
	// 创建纹理
	// OpenGL纹理坐标和DirectX不同，颠倒一下
	flipVertically(
		bitmapGlyph.buffer,
		bitmapGlyph.width,
		bitmapGlyph.rows
	);

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