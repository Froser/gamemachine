#include "stdafx.h"
#include "gmimagereader.h"
#include "gmimagereader_bmp.h"
#include <map>
#include "gmimagereader_png.h"
#include "gmimagereader_jpg.h"
#include "gmimagereader_tga.h"
#include "gmimagereader_tiff.h"

BEGIN_NS

namespace
{
	GMImageReader::ImageType test(const GMbyte* data, GMsize_t size)
	{
		for (GMImageReader::ImageType i = GMImageReader::ImageType_Begin; i < GMImageReader::ImageType_End; i = (GMImageReader::ImageType)((GMuint32)i + 1))
		{
			if (GMImageReader::getReader(i)->test(data, size))
				return i;
		}
		return GMImageReader::ImageType_End;
	}
}

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

END_NS