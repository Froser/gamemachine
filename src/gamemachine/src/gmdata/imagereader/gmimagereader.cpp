#include "stdafx.h"
#include "gmimagereader.h"
#include "gmimagereader_bmp.h"
#include <map>
#include "gmimagereader_png.h"
#include "gmimagereader_jpg.h"
#include "gmimagereader_tga.h"
#include "gmimagereader_tiff.h"

class GMImageReaderContainer
{
public:
	GMImageReaderContainer()
	{
		m_readers[GMImageReader::ImageType_BMP] = new GMImageReader_BMP();
		m_readers[GMImageReader::ImageType_PNG] = new GMImageReader_PNG();
		m_readers[GMImageReader::ImageType_JPG] = new GMImageReader_JPG();
		m_readers[GMImageReader::ImageType_TIFF] = new GMImageReader_TIFF();
		m_readers[GMImageReader::ImageType_TGA] = new GMImageReader_TGA();
	}

	~GMImageReaderContainer()
	{
		for (auto iter = m_readers.begin(); iter != m_readers.end(); iter++)
		{
			delete iter->second;
		}
	}

	IImageReader* getReader(GMImageReader::ImageType type)
	{
		GM_ASSERT(m_readers.find(type) != m_readers.end());
		return m_readers[type];
	}

private:
	Map<GMImageReader::ImageType, IImageReader*> m_readers;
};

bool GMImageReader::load(const GMbyte* data, GMsize_t size, OUT GMImage** image)
{
	return load(data, size, ImageType_AUTO, image);
}

bool GMImageReader::load(const GMbyte* data, GMsize_t size, ImageType type, OUT GMImage** image)
{
	if (type == ImageType_AUTO)
		type = test(data, size);

	if (type == ImageType_End)
		return false;

	return getReader(type)->load(data, size, image);
}

IImageReader* GMImageReader::getReader(ImageType type)
{
	static GMImageReaderContainer readers;
	return readers.getReader(type);
}

GMImageReader::ImageType GMImageReader::test(const GMbyte* data, GMsize_t size)
{
	for (ImageType i = ImageType_Begin; i < ImageType_End; i = (ImageType)((GMuint32)i + 1))
	{
		if (getReader(i)->test(data, size))
			return i;
	}
	return ImageType_End;
}