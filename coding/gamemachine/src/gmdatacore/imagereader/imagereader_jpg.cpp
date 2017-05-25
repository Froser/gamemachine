#include "stdafx.h"
#include "imagereader_jpg.h"
#include <iostream>
#include <fstream>

extern "C"
{
	#include "jpeglib.h"

	/* Read JPEG image from a memory segment */
	static void init_source(j_decompress_ptr cinfo) {}
	static boolean fill_input_buffer(j_decompress_ptr cinfo)
	{
		return TRUE;
	}

	static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
	{
		struct jpeg_source_mgr* src = (struct jpeg_source_mgr*) cinfo->src;

		if (num_bytes > 0) {
			src->next_input_byte += (size_t)num_bytes;
			src->bytes_in_buffer -= (size_t)num_bytes;
		}
	}
	static void term_source(j_decompress_ptr cinfo) {}
	static void jpeg_mem_src(j_decompress_ptr cinfo, void* buffer, long nbytes)
	{
		struct jpeg_source_mgr* src;

		if (cinfo->src == NULL) {   /* first time for this JPEG object? */
			cinfo->src = (struct jpeg_source_mgr *)
				(*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_PERMANENT,
				sizeof(struct jpeg_source_mgr));
		}

		src = (struct jpeg_source_mgr*) cinfo->src;
		src->init_source = init_source;
		src->fill_input_buffer = fill_input_buffer;
		src->skip_input_data = skip_input_data;
		src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
		src->term_source = term_source;
		src->bytes_in_buffer = nbytes;
		src->next_input_byte = (JOCTET*)buffer;
	}
}

bool ImageReader_JPG::load(const GMbyte* data, GMuint size, OUT Image** image)
{
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
	jpeg_mem_src(&cinfo, (void*)data, size);

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

	GMuint bufferSize = imgData.mip[0].width * imgData.mip[0].height * channels;
	imgData.mip[0].data = new GMbyte[bufferSize];
	imgData.size = bufferSize;

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

	//delete row pointers
	if (rowPtr)
		delete[] rowPtr;

	img->flipVertically(0);

	return true;
}

bool ImageReader_JPG::test(const GMbyte* data)
{
	const short* p = reinterpret_cast<const short*>(data);
	return *p == (short) 0xd8ff;
}
