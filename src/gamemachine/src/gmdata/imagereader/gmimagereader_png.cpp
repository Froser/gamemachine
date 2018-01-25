#include "stdafx.h"
#include "gmimagereader_png.h"
#include <fstream>
#include <gmimage.h>
#include "png.h"
#include <GL/glew.h>

#define PNG_BYTES_TO_CHECK 4
struct PngImage
{
	const GMbyte* data;
	GMuint size;
	GMuint offset;
};

namespace
{
	void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		PngImage* isource = (PngImage*)png_get_io_ptr(png_ptr);
		if (isource->offset + length <= isource->size)
		{
			memcpy(data, isource->data + isource->offset, length);
			isource->offset += length;
		}
		else
			png_error(png_ptr, "pngReaderCallback failed");
	}

	bool loadPng(const GMbyte* data, GMuint dataSize, PngData *out, REF GMuint& bufferSize, bool flip = false)
	{
		png_structp png_ptr;
		png_infop  info_ptr;

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		info_ptr = png_create_info_struct(png_ptr);
		setjmp(png_jmpbuf(png_ptr));

		GMint temp = png_sig_cmp((png_const_bytep)data, (png_size_t)0, PNG_BYTES_TO_CHECK);
		// 检测是否为png文件
		GM_ASSERT(temp == 0);
		PngImage imgsource = { data, dataSize, 0 };
		png_set_read_fn(png_ptr, &imgsource, pngReadCallback);
		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

		int color_type, channels;

		channels = png_get_channels(png_ptr, info_ptr);
		out->bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);

		int i, j;
		int size, pos = -1;

		png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
		out->width = png_get_image_width(png_ptr, info_ptr);
		out->height = png_get_image_height(png_ptr, info_ptr);
		size = out->width * out->height;

		// 根据flip来决定是否翻转Y轴
		if (channels == 4 || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		{
			size *= (4 * sizeof(unsigned char));
			out->hasAlpha = true;
			out->rgba = new GMbyte[size];

			temp = (4 * out->width);
			for (i = 0; i < out->height; i++)
			{
				int _i = i;
				if (flip)
					_i = out->height - i - 1;

				for (j = 0; j < temp; j += 4)
				{
					out->rgba[++pos] = row_pointers[_i][j]; // red
					out->rgba[++pos] = row_pointers[_i][j + 1]; // green
					out->rgba[++pos] = row_pointers[_i][j + 2];  // blue
					out->rgba[++pos] = row_pointers[_i][j + 3]; // alpha
				}
			}
		}
		else if (channels == 3 || color_type == PNG_COLOR_TYPE_RGB)
		{
			size *= (3 * sizeof(GMbyte));
			out->hasAlpha = false;
			out->rgba = new GMbyte[size];

			temp = (3 * out->width);
			for (i = 0; i < out->height; i++)
			{
				int _i = i;
				if (flip)
					_i = out->height - i - 1;

				for (j = 0; j < temp; j += 3)
				{
					out->rgba[++pos] = row_pointers[_i][j]; // red
					out->rgba[++pos] = row_pointers[_i][j + 1]; // green
					out->rgba[++pos] = row_pointers[_i][j + 2];  // blue
				}
			}
		}
		else
		{
			return false;
		}

		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		bufferSize = size;
		return true;
	}
}

struct PNGTestHeader
{
	DWORD magic1, magic2;
};

bool GMImageReader_PNG::load(const GMbyte* data, GMuint size, OUT GMImage** img)
{
	GM_ASSERT(img);
	*img = new GMImage();

	PngData png;
	GMuint bufferSize;
	bool b = loadPng(data, size, &png, bufferSize, true);
	writeDataToImage(png, *img, bufferSize);
	return b;
}

bool GMImageReader_PNG::test(const GMbyte* data)
{
	const PNGTestHeader* header = reinterpret_cast<const PNGTestHeader*>(data);
	return header && header->magic1 == 1196314761 && header->magic2 == 169478669;
}

void GMImageReader_PNG::writeDataToImage(PngData& png, GMImage* img, GMuint size)
{
	GM_ASSERT(img);
	GMImage::Data& data = img->getData();
#if GM_USE_OPENGL
	data.target = GL_TEXTURE_2D;
	data.mipLevels = 1;
	if (png.hasAlpha)
	{
		data.internalFormat = GL_RGBA8;
		data.format = GL_RGBA;
	}
	else
	{
		data.internalFormat = GL_RGB8;
		data.format = GL_RGB;
	}

	data.swizzle[0] = GL_RED;
	data.swizzle[1] = GL_GREEN;
	data.swizzle[2] = GL_BLUE;
	data.swizzle[3] = GL_ALPHA;
	data.type = GL_UNSIGNED_BYTE;
	data.mip[0].height = png.height;
	data.mip[0].width = png.width;
	// Buffer 移交给 Image 管理
	data.mip[0].data = png.rgba;
	data.size = size;
#else
	GM_ASSERT(false);
#endif
}