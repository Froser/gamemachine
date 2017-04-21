#include "stdafx.h"
#include "imagereader_png.h"
#include <fstream>
#include "gmdatacore/image.h"
#include "png.h"
#include "utilities/assert.h"

#define PNG_BYTES_TO_CHECK 4

bool loadPng(const GMbyte* data, PngData *out, bool flip = false)
{
	FILE *pic_fp = nullptr;
	fopen_s(&pic_fp, filepath, "rb");
	if (pic_fp == nullptr)
		return false;

	png_structp png_ptr;
	png_infop  info_ptr;
	char buf[PNG_BYTES_TO_CHECK];
	int temp;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);

	setjmp(png_jmpbuf(png_ptr));

	temp = fread(buf, 1, PNG_BYTES_TO_CHECK, pic_fp);
	temp = png_sig_cmp((png_const_bytep) buf, (png_size_t)0, PNG_BYTES_TO_CHECK);

	// 检测是否为png文件
	ASSERT(temp == 0);

	rewind(pic_fp);
	png_init_io(png_ptr, pic_fp);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

	int color_type, channels;

	channels = png_get_channels(png_ptr, info_ptr);
	out->bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	int i, j;
	int size, pos = 0;
	
	png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
	out->width = png_get_image_width(png_ptr, info_ptr);
	out->height = png_get_image_height(png_ptr, info_ptr);
	size = out->width * out->height;

	// 根据flip来决定是否翻转Y轴

	if (channels == 4 || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		size *= (4 * sizeof(unsigned char));
		out->hasAlpha = true;
		out->rgba = new unsigned char[size];

		temp = (4 * out->width);
		for (i = 0; i < out->height; i++)
		{
			int _i = i;
			if (flip)
				_i = out->height - i - 1;

			for (j = 0; j < temp; j += 4)
			{
				out->rgba[pos++] = row_pointers[_i][j]; // red
				out->rgba[pos++] = row_pointers[_i][j + 1]; // green
				out->rgba[pos++] = row_pointers[_i][j + 2];  // blue
				out->rgba[pos++] = row_pointers[_i][j + 3]; // alpha
			}
		}
	}
	else if (channels == 3 || color_type == PNG_COLOR_TYPE_RGB)
	{
		size *= (3 * sizeof(unsigned char));
		out->hasAlpha = false;
		out->rgba = new unsigned char[size];

		temp = (3 * out->width);
		for (i = 0; i < out->height; i++)
		{
			int _i = i;
			if (flip)
				_i = out->height - i - 1;

			for (j = 0; j < temp; j += 3)
			{
				out->rgba[pos++] = row_pointers[i][j]; // red
				out->rgba[pos++] = row_pointers[i][j + 1]; // green
				out->rgba[pos++] = row_pointers[i][j + 2];  // blue
			}
		}
	}
	else
	{
		return false;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return true;
}
struct PNGTestHeader
{
	DWORD magic1, magic2;
};

bool ImageReader_PNG::load(const GMbyte* data, OUT Image** img)
{
	ASSERT(img);
	*img = new Image();

	PngData png;
	bool b = loadPng(data, &png, true);
	writeDataToImage(png, *img);
	return b;
}

bool ImageReader_PNG::test(const GMbyte* data)
{
	std::ifstream file;
	const PNGTestHeader* header = reinterpret_cast<const PNGTestHeader*>(data);
	return header->magic1 == 1196314761 && header->magic2 == 169478669;
}

void ImageReader_PNG::writeDataToImage(PngData& png, Image* img)
{
	ASSERT(img);
	ImageData& data = img->getData();
#ifdef USE_OPENGL
	data.target = GL_TEXTURE_2D;
	data.mipLevels = 1;
	data.internalFormat = GL_RGB16;
	if (png.hasAlpha)
		data.format = GL_RGBA;
	else
		data.format = GL_RGB;

	data.swizzle[0] = GL_RED;
	data.swizzle[1] = GL_GREEN;
	data.swizzle[2] = GL_BLUE;
	data.swizzle[3] = GL_ALPHA;
	data.type = GL_UNSIGNED_BYTE;
	data.mip[0].height = png.height;
	data.mip[0].width = png.width;
	// Buffer 移交给 Image 管理
	data.mip[0].data = png.rgba;
#else
	ASSERT(false);
#endif
}