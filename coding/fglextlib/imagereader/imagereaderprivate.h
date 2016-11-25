#ifndef __IMAGEREADER_PRIVATE_H__
#include "common.h"
#include <string>
BEGIN_NS

class ImageReaderPrivate
{
	friend class ImageReader;

private:
	void setFilename(const char* filename);
	void load();

private:
	std::string m_filename;
};

END_NS
#endif