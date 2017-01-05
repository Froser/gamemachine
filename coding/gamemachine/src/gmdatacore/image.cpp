#include "stdafx.h"
#include "image.h"

Image::Image()
{
	memset(&m_data, 0, sizeof(m_data));
}

Image::~Image()
{
	dispose();
}

ImageData& Image::getData()
{
	return m_data;
}

void Image::dispose()
{
	if (m_data.mip[0].data)
	{
		delete[] reinterpret_cast<GMbyte *>(m_data.mip[0].data);
		m_data.mip[0].data = nullptr;
	}
}