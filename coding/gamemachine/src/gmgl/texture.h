#ifndef __GMGL_TEXTURE_H__
#define __GMGL_TEXTURE_H__
#include "common.h"
#include "gmdatacore/image.h"
#include "utilities/autoptr.h"

BEGIN_NS

class GMGLTexture
{
public:
	static void loadTexture(const char* filename, GMGLTexture& texture);

public:
	~GMGLTexture();

private:
	AutoPtr<Image> m_image;
	GLuint m_id;
};

END_NS
#endif