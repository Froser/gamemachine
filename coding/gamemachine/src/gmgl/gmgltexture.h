#ifndef __GMGL_TEXTURE_H__
#define __GMGL_TEXTURE_H__
#include <gmcommon.h>
#include <gmimage.h>
#include "foundation/utilities/utilities.h"
#include "gmdatacore/gmmodel.h"
#include <map>
#include <string>
#include "shader_constants.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMGLTexture)
{
	bool inited = false;
	GMImage* image = nullptr;
	GLuint id = 0;
};

class GMGLTexture : public ITexture
{
	DECLARE_PRIVATE(GMGLTexture)

public:
	GMGLTexture(AUTORELEASE GMImage* image);
	~GMGLTexture();

public:
	void init();

public:
	virtual void drawTexture(GMTextureFrames* frames) override;
};

END_NS
#endif