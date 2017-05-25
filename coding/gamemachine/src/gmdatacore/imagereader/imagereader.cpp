#include "stdafx.h"
#include "imagereader.h"
#include "imagereader_bmp.h"
#include "imagereader_dds.h"
#include <map>
#include "imagereader_png.h"
#include "imagereader_tga.h"
#include "imagereader_jpg.h"

class ImageReaderContainer
{
public:
	ImageReaderContainer()
	{
		m_readers[ImageType_BMP] = new ImageReader_BMP();
		m_readers[ImageType_DDS] = new ImageReader_DDS();
		m_readers[ImageType_PNG] = new ImageReader_PNG();
		m_readers[ImageType_TGA] = new ImageReader_TGA();
		m_readers[ImageType_JPG] = new ImageReader_JPG();
	}

	~ImageReaderContainer()
	{
		for (auto iter = m_readers.begin(); iter != m_readers.end(); iter++)
		{
			delete iter->second;
		}
	}

	IImageReader* getReader(ImageType type)
	{
		ASSERT(m_readers.find(type) != m_readers.end());
		return m_readers[type];
	}

private:
	std::map<ImageType, IImageReader*> m_readers;
};

bool ImageReader::load(const GMbyte* data, GMuint size, OUT Image** image)
{
	return load(data, size, ImageType_AUTO, image);
}

bool ImageReader::load(const GMbyte* data, GMuint size, ImageType type, OUT Image** image)
{
	if (type == ImageType_AUTO)
		type = test(data);

	if (type == ImageType_End)
		return false;

	return getReader(type)->load(data, size, image);
}

IImageReader* ImageReader::getReader(ImageType type)
{
	static ImageReaderContainer readers;
	return readers.getReader(type);
}

ImageType ImageReader::test(const GMbyte* data)
{
	for (ImageType i = ImageType_Begin; i < ImageType_End; i = (ImageType)((GMuint)i + 1))
	{
		if (getReader(i)->test(data))
			return i;
	}
	return ImageType_End;
}