#include "stdafx.h"
#include "imagereader.h"
#include "utilities/assert.h"
#include <fstream>

void ImageReaderPrivate::load(const char* filename, Image* img)
{
	std::ifstream file;
	file.open(filename, std::ios::in | std::ios::binary);
	if (file.good())
	{
		BitmapFile& bitmapFile = img->getRawFile();

		file.read(reinterpret_cast<char*>(&bitmapFile.bitmapHeader), sizeof(BitmapHeader));
		file.read(reinterpret_cast<char*>(&bitmapFile.bitmapInfoHeader), sizeof(BitmapInfoHeader));

		long paletteLen = bitmapFile.bitmapHeader.bfOffBits - sizeof(BitmapHeader) - sizeof(BitmapInfoHeader);
		file.read(reinterpret_cast<char*>(&bitmapFile.palette), paletteLen);

		ASSERT(bitmapFile.bitmapInfoHeader.biHeight > 0
			&& bitmapFile.bitmapInfoHeader.biBitCount == 24
			&& bitmapFile.bitmapInfoHeader.biCompression == 0);

		long cnt = bitmapFile.bitmapInfoHeader.biWidth * bitmapFile.bitmapInfoHeader.biHeight * 3;
		bitmapFile.buffer = new BYTE[cnt];
		file.read(reinterpret_cast<char*>(bitmapFile.buffer), cnt);
	}
}

ImagePrivate::~ImagePrivate()
{
	delete[] m_bitmapFile.buffer;
}