#ifndef __GMGL_TEXTURE_H__
#define __GMGL_TEXTURE_H__
#include <gmcommon.h>
#include <gmimage.h>
#include "foundation/utilities/utilities.h"
#include "gmdata/gmmodel.h"
#include <map>
#include <string>
#include "shader_constants.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMGLTexture)
{
	bool inited = false;
	GLuint id = 0;
	GLenum target = 0;
};

class GMGLTexture : public ITexture
{
	DECLARE_PRIVATE_NGO(GMGLTexture)

public:
	GMGLTexture(const GMImage* image);
	~GMGLTexture();

public:
	void init(const GMImage* image);

public:
	virtual void drawTexture(GMTextureFrames* frames) override;
};

END_NS
#endif