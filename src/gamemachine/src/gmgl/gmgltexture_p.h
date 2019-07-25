#ifndef __GMGL_TEXTURE_P_H__
#define __GMGL_TEXTURE_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGLTexture)
{
	bool inited = false;
	GMuint32 id = 0;
	GMuint32 target = 0;
	GMuint32 format = 0;
	GMuint32 internalFormat = 0;
	GMuint32 dataType = 0;
	const GMImage* image = nullptr;
	bool texParamsSet = false;
};

END_NS
#endif