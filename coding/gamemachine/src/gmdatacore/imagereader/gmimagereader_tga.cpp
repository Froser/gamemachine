#include "stdafx.h"
#include <iostream>
#include <string.h>
#include <time.h>
#include <math.h>
#include "gmimagereader_tga.h"
#include "foundation/utilities/utilities.h"
#include <fstream>

bool endsWith(const std::string& str, const std::string& substr)
{
	return str.rfind(substr) == (str.length() - substr.length());
}

TGAImage::TGAImage() : d(nullptr), width(0), height(0), bytespp(0), nbytes(0) {}

TGAImage::TGAImage(int w, int h, int bpp) : d(nullptr), width(w), height(h), bytespp(bpp) {
	nbytes = width*height*bytespp;
	d = new unsigned char[nbytes];
	memset(d, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage &img) : d(nullptr), width(img.width), height(img.height), bytespp(img.bytespp)
{
	unsigned long nbytes = width*height*bytespp;
	d = new unsigned char[nbytes];
	memcpy(d, img.d, nbytes);
}

TGAImage::~TGAImage()
{
	if (d)
		delete [] d;
}

TGAImage & TGAImage::operator =(const TGAImage &img)
{
	if (this != &img) {
		if (d) delete [] d;
		width  = img.width;
		height = img.height;
		bytespp = img.bytespp;
		unsigned long nbytes = width*height*bytespp;
		d = new unsigned char[nbytes];
		memcpy(d, img.d, nbytes);
	}
	return *this;
}

bool TGAImage::read_tga_file(const GMbyte* data, GMuint size)
{
	if (d)
		delete [] d;
	d = nullptr;

	MemoryStream in(data, size);
	TGA_Header header;
	in.read(reinterpret_cast<GMbyte*>(&header), sizeof(header));

	width   = header.width;
	height  = header.height;
	bytespp = header.bitsperpixel>>3;
	if (width<=0 || height<=0 || (bytespp!=GRAYSCALE && bytespp!=RGB && bytespp!=RGBA)) {
		gm_error("bad bpp (or width/height) value");
		return false;
	}
	nbytes = bytespp * width * height;
	d = new GMbyte[nbytes];
	if (3==header.datatypecode || 2==header.datatypecode)
	{
		in.read(reinterpret_cast<GMbyte*>(d), nbytes);
	}
	else if (10==header.datatypecode||11==header.datatypecode)
	{
		if (!load_rle_data(in))
		{
			gm_error("an error occured while reading the data");
			return false;
		}
	}
	else 
	{
		ASSERT(false);
		gm_error("unknown file format %d", (int)header.datatypecode);
		return false;
	}
	if (!(header.imagedescriptor & 0x20))
		flip_vertically();
	if (header.imagedescriptor & 0x10)
		flip_horizontally();
	return true;
}

bool TGAImage::load_rle_data(MemoryStream& in) {
	unsigned long pixelcount = width*height;
	unsigned long currentpixel = 0;
	unsigned long currentbyte  = 0;
	TGAColor colorbuffer;
	do {
		unsigned char chunkheader = 0;
		chunkheader = in.get();

		if (chunkheader<128) {
			chunkheader++;
			for (int i=0; i<chunkheader; i++)
			{
				in.read(reinterpret_cast<GMbyte*>(colorbuffer.bgra), bytespp);
				for (GMint t=0; t<bytespp; t++)
					d[currentbyte++] = colorbuffer.bgra[t];
				currentpixel++;
				if (currentpixel>pixelcount) {
					gm_error("Too many pixels read");
					return false;
				}
			}
		} else {
			chunkheader -= 127;
			in.read(reinterpret_cast<GMbyte*>(colorbuffer.bgra), bytespp);
			for (int i=0; i<chunkheader; i++)
			{
				for (int t=0; t<bytespp; t++)
					d[currentbyte++] = colorbuffer.bgra[t];
				currentpixel++;
				if (currentpixel>pixelcount) {
					gm_error("Too many pixels read");
					return false;
				}
			}
		}
	} while (currentpixel < pixelcount);
	return true;
}

bool TGAImage::write_tga_file(const char *filename, bool rle) const {
	unsigned char developer_area_ref[4] = {0, 0, 0, 0};
	unsigned char extension_area_ref[4] = {0, 0, 0, 0};
	unsigned char footer[18] = {'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0'};
	std::ofstream out;
	out.open (filename, std::ios::binary);
	if (!out.is_open()) {
		gm_error("can't open file %s", filename);
		out.close();
		return false;
	}
	TGA_Header header;
	memset((void *)&header, 0, sizeof(header));
	header.bitsperpixel = bytespp<<3;
	header.width  = width;
	header.height = height;
	header.datatypecode = (bytespp==GRAYSCALE?(rle?11:3):(rle?10:2));
	header.imagedescriptor = 0x20; // top-left origin
	out.write((char *)&header, sizeof(header));
	if (!out.good()) {
		out.close();
		gm_error("can't dump the tga file");
		return false;
	}
	if (!rle) {
		out.write((char *)d, width*height*bytespp);
		if (!out.good()) {
			gm_error("can't unload raw data");
			out.close();
			return false;
		}
	} else {
		if (!unload_rle_data(out)) {
			out.close();
			gm_error("can't unload rle data");
			return false;
		}
	}
	out.write((char *)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good()) {
		gm_error("can't dump the tga file");
		out.close();
		return false;
	}
	out.write((char *)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good()) {
		gm_error("can't dump the tga file");
		out.close();
		return false;
	}
	out.write((char *)footer, sizeof(footer));
	if (!out.good()) {
		gm_error("can't dump the tga file");
		out.close();
		return false;
	}
	out.close();
	return true;
}

// TODO: it is not necessary to break a raw chunk for two equal pixels (for the matter of the resulting size)
bool TGAImage::unload_rle_data(std::ofstream &out) const {
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = width*height;
	unsigned long curpix = 0;
	while (curpix<npixels) {
		unsigned long chunkstart = curpix*bytespp;
		unsigned long curbyte = curpix*bytespp;
		unsigned char run_length = 1;
		bool raw = true;
		while (curpix+run_length<npixels && run_length<max_chunk_length) {
			bool succ_eq = true;
			for (int t=0; succ_eq && t<bytespp; t++) {
				succ_eq = (d[curbyte+t]==d[curbyte+t+bytespp]);
			}
			curbyte += bytespp;
			if (1==run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw?run_length-1:run_length+127);
		if (!out.good()) {
			gm_error("can't dump the tga file");
			return false;
		}
		out.write((char *)(d+chunkstart), (raw?run_length*bytespp:bytespp));
		if (!out.good()) {
			gm_error("can't dump the tga file");
			return false;
		}
	}
	return true;
}

TGAColor TGAImage::get(int x, int y) const {
	if (!d || x<0 || y<0 || x>=width || y>=height) {
		return TGAColor(128.f,128.f,128.f,255.f);
	}
	return TGAColor(d+(x+y*width)*bytespp, bytespp);
}

bool TGAImage::set(int x, int y, TGAColor &c) {
	if (!d || x<0 || y<0 || x>=width || y>=height) {
		return false;
	}
	memcpy(d+(x+y*width)*bytespp, c.bgra, bytespp);
	return true;
}

bool TGAImage::set(int x, int y, const TGAColor &c) {
	if (!d || x<0 || y<0 || x>=width || y>=height) {
		return false;
	}
	memcpy(d+(x+y*width)*bytespp, c.bgra, bytespp);
	return true;
}

int TGAImage::get_bytespp() {
	return bytespp;
}

int TGAImage::get_width() {
	return width;
}

int TGAImage::get_height() {
	return height;
}

size_t TGAImage::get_nbytes()
{
	return nbytes;
}

bool TGAImage::flip_horizontally() {
	if (!d) return false;
	int half = width>>1;
	for (int i=0; i<half; i++) {
		for (int j=0; j<height; j++) {
			TGAColor c1 = get(i, j);
			TGAColor c2 = get(width-1-i, j);
			set(i, j, c2);
			set(width-1-i, j, c1);
		}
	}
	return true;
}

bool TGAImage::flip_vertically() {
	if (!d) return false;
	unsigned long bytes_per_line = width*bytespp;
	unsigned char *line = new unsigned char[bytes_per_line];
	int half = height>>1;
	for (int j=0; j<half; j++) {
		unsigned long l1 = j*bytes_per_line;
		unsigned long l2 = (height-1-j)*bytes_per_line;
		memmove((void *)line,      (void *)(d+l1), bytes_per_line);
		memmove((void *)(d+l1), (void *)(d+l2), bytes_per_line);
		memmove((void *)(d+l2), (void *)line,      bytes_per_line);
	}
	delete [] line;
	return true;
}

unsigned char *TGAImage::buffer() {
	return d;
}

void TGAImage::clear() {
	memset((void *)d, 0, width*height*bytespp);
}

bool TGAImage::scale(int w, int h) {
	if (w<=0 || h<=0 || !d) return false;
	unsigned char *tdata = new unsigned char[w*h*bytespp];
	int nscanline = 0;
	int oscanline = 0;
	int erry = 0;
	unsigned long nlinebytes = w*bytespp;
	unsigned long olinebytes = width*bytespp;
	for (int j=0; j<height; j++) {
		int errx = width-w;
		int nx   = -bytespp;
		int ox   = -bytespp;
		for (int i=0; i<width; i++) {
			ox += bytespp;
			errx += w;
			while (errx>=(int)width) {
				errx -= width;
				nx += bytespp;
				memcpy(tdata+nscanline+nx, d+oscanline+ox, bytespp);
			}
		}
		erry += h;
		oscanline += olinebytes;
		while (erry>=(int)height) {
			if (erry>=(int)height<<1) // it means we jump over a scanline
				memcpy(tdata+nscanline+nlinebytes, tdata+nscanline, nlinebytes);
			erry -= height;
			nscanline += nlinebytes;
		}
	}
	delete [] d;
	d = tdata;
	width = w;
	height = h;
	return true;
}


bool GMImageReader_TGA::load(const GMbyte* data, GMuint size, OUT GMImage** img)
{
	ASSERT(img);
	*img = new GMImage();
	TGAImage tga;
	bool b = tga.read_tga_file(data, size);
	if (b)
	{
		tga.flip_vertically();
		writeDataToImage(tga, *img);
	}
	return b;
}

bool GMImageReader_TGA::test(const GMbyte* data)
{
	return data[0] == 0 && data[1] == 0;
}

void GMImageReader_TGA::writeDataToImage(TGAImage& tga, GMImage* img)
{
	ASSERT(img);
	GMImage::Data& data = img->getData();
#if USE_OPENGL
	data.target = GL_TEXTURE_2D;
	data.mipLevels = 1;
	data.internalFormat = GL_RGB16;

	if (tga.get_bytespp() == 3)
		data.format = GL_BGR;
	else if (tga.get_bytespp() == 4)
		data.format = GL_BGRA;
	else
		gm_error("不支持16位的tga");

	data.swizzle[0] = GL_RED;
	data.swizzle[1] = GL_GREEN;
	data.swizzle[2] = GL_BLUE;
	data.swizzle[3] = GL_ALPHA;
	data.type = GL_UNSIGNED_BYTE;
	data.size = tga.get_nbytes();
	data.mip[0].height = tga.get_height();
	data.mip[0].width = tga.get_width();
	data.mip[0].data = new GMbyte[tga.get_nbytes()];
	memcpy(data.mip[0].data, tga.buffer(), tga.get_nbytes());
#else
	ASSERT(false);
#endif
}