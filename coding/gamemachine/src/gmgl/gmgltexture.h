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

class GMGLTexture : public ITexture
{
public:
	GMGLTexture(AUTORELEASE GMImage* image);
	~GMGLTexture();

public:
	void init();

public:
	virtual void drawTexture(GMTextureFrames* frames) override;

private:
	bool m_inited;
	AutoPtr<GMImage> m_image;
	GLuint m_id;
};

END_NS
#endif