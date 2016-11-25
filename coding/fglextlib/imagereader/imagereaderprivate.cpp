#include "stdafx.h"
#include "imagereader.h"

void ImageReaderPrivate::setFilename(const char* filename)
{
	m_filename = filename;
}

void ImageReaderPrivate::load()
{
#ifdef _WINDOWS
	HBITMAP h = (HBITMAP)::LoadImage(NULL, m_filename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (h)
	{
		BITMAP bitmap = { 0 };
		int ret = GetObject(h, sizeof(BITMAP), &bitmap);
		if (ret)
		{
			BYTE* pPixel = (BYTE*)bitmap.bmBits;
		}
	}
	DeleteObject(h);
#endif
}