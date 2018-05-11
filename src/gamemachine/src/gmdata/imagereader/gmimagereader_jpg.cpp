#include "stdafx.h"
#include "gmimagereader_jpg.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>

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

bool GMImageReader_JPG::load(const GMbyte* data, GMsize_t size, OUT GMImage** image)
{
	GMImage* img = new GMImage();
	*image = img;
	GMImage::Data& imgData = img->getData();
	//init image data
	imgData.target = GMImageTarget::Texture2D;
	imgData.mipLevels = 1;
	imgData.swizzle[0] = GL_RED;
	imgData.swizzle[1] = GL_GREEN;
	imgData.swizzle[2] = GL_BLUE;
	imgData.swizzle[3] = GL_ALPHA;
	imgData.type = GMImageDataType::UnsignedByte;

	//Create struct
	struct jpeg_decompress_struct cinfo;

	//Create an error handler
	jpeg_error_mgr jerr;

	//point the compression object to the error handler
	cinfo.err = jpeg_std_error(&jerr);

	//Initialize the decompression object
	jpeg_create_decompress(&cinfo);

	//Specify the data source
	jpeg_mem_src(&cinfo, (void*)data, (long)size);

	//Decode the jpeg data into the image
	//Read in the header
	jpeg_read_header(&cinfo, true);

	//start to decompress the data
	jpeg_start_decompress(&cinfo);

	//get the number of color channels
	int channels = cinfo.num_components;

	imgData.mip[0].width = cinfo.image_width;
	imgData.mip[0].height = cinfo.image_height;
	imgData.format = GMImageFormat::RGBA;
	imgData.internalFormat = GMImageInternalFormat::RGBA8;

	GMuint bufferSize = cinfo.image_width * cinfo.image_height * GMImageReader::DefaultChannels;
	imgData.mip[0].data = new GMbyte[bufferSize];
	imgData.size = bufferSize;

	//Extract the pixel data
	GMbyte** rowPtr = nullptr;
	int rowsRead = 0;
	if (channels == 3)
	{
		//If channel number is 3, we have to make up a 4-channel image data
		size_t bytesSize = cinfo.image_width * cinfo.image_height * channels;
		GMbyte* tempData = new GMbyte[bytesSize];
		rowPtr = new GMbyte*[cinfo.image_height];
		for (GMuint i = 0; i < cinfo.image_height; ++i)
			rowPtr[i] = &(tempData[i * cinfo.image_width * 3]);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			//read in this row
			rowsRead += jpeg_read_scanlines(&cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead);
		}

		GMuint dataPtr = 0;
		for (GMuint i = 0; i < bytesSize; ++i, ++dataPtr)
		{
			imgData.mip[0].data[dataPtr] = tempData[i];
			if ((i + 1) % 3 == 0)
			{
				imgData.mip[0].data[++dataPtr] = 0xFF; //ALPHA
			}
		}

		GM_delete_array(tempData);
	}
	else
	{
		rowPtr = new GMbyte*[cinfo.image_height];
		for (GMuint i = 0; i < cinfo.image_height; ++i)
			rowPtr[i] = &(imgData.mip[0].data[i * cinfo.image_width * channels]);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			//read in this row
			rowsRead += jpeg_read_scanlines(&cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead);
		}

		imgData.channels = channels;
		if (channels == GMImageReader::DefaultChannels)
		{
			imgData.format = GMImageFormat::RGBA;
			imgData.internalFormat = GMImageInternalFormat::RGBA8;
		}
		else if (channels == 1)
		{
			imgData.format = GMImageFormat::RED;
			imgData.internalFormat = GMImageInternalFormat::RED8;
		}
		else
		{
			GM_ASSERT(false);
		}
	}
	//release memory used by jpeg
	jpeg_destroy_decompress(&cinfo);

	//delete row pointers
	GM_delete_array(rowPtr);
	return true;
}

bool GMImageReader_JPG::test(const GMbyte* data)
{
	const short* p = reinterpret_cast<const short*>(data);
	return p && *p == (short) 0xd8ff;
}
