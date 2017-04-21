#include "stdafx.h"
#include "imagereader_jpg.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

extern "C"
{
	#include "jpeglib.h"
}

bool ImageReader_JPG::load(const GMbyte* data, OUT Image** image)
{
	FILE* file = nullptr;
	fopen_s(&file, filename, "rb");
	if (!file)
		return false;

	Image* img = new Image();
	*image = img;
	ImageData& imgData = img->getData();
	//init image data
	imgData.target = GL_TEXTURE_2D;
	imgData.mipLevels = 1;
	imgData.format = GL_RGB16;
	imgData.swizzle[0] = GL_RED;
	imgData.swizzle[1] = GL_GREEN;
	imgData.swizzle[2] = GL_BLUE;
	imgData.swizzle[3] = GL_ALPHA;
	imgData.type = GL_UNSIGNED_BYTE;

	//Create struct
	struct jpeg_decompress_struct cinfo;

	//Create an error handler
	jpeg_error_mgr jerr;

	//point the compression object to the error handler
	cinfo.err = jpeg_std_error(&jerr);

	//Initialize the decompression object
	jpeg_create_decompress(&cinfo);

	//Specify the data source
	jpeg_stdio_src(&cinfo, file);

	//Decode the jpeg data into the image
	//Read in the header
	jpeg_read_header(&cinfo, true);

	//start to decompress the data
	jpeg_start_decompress(&cinfo);

	//get the number of color channels
	int channels = cinfo.num_components;

	imgData.mip[0].depth = channels * 8;
	imgData.mip[0].width = cinfo.image_width;
	imgData.mip[0].height = cinfo.image_height;

	if (imgData.mip[0].depth == 32)
	{
		imgData.format = GL_RGBA;
		imgData.internalFormat = GL_RGBA8;
	}
	else
	{
		imgData.format = GL_RGB;
		imgData.internalFormat = GL_RGB8;
	}

	imgData.mip[0].data = new GMbyte[imgData.mip[0].width * imgData.mip[0].height * channels];

	GMbyte** rowPtr = new GMbyte*[imgData.mip[0].height];

	for (GMint i = 0; i < imgData.mip[0].height; ++i)
		rowPtr[i] = &(imgData.mip[0].data[i * imgData.mip[0].width * channels]);

	//Extract the pixel data
	int rowsRead = 0;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		//read in this row
		rowsRead += jpeg_read_scanlines(&cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead);
	}

	//release memory used by jpeg
	jpeg_destroy_decompress(&cinfo);

	fclose(file);

	//delete row pointers
	if (rowPtr)
		delete[] rowPtr;

	img->flipVertically(0);

	return true;
}

bool ImageReader_JPG::test(const GMbyte* data)
{
	return data[1] == (char) 0xff;
}
