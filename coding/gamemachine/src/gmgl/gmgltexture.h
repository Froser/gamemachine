#ifndef __GMGL_TEXTURE_H__
#define __GMGL_TEXTURE_H__
#include "common.h"
#include "gmdatacore/image.h"
#include "gmdatacore/texture.h"
#include "utilities/autoptr.h"

BEGIN_NS

class GMGLTexture : public ITexture
{
public:
	GMGLTexture(AUTORELEASE Image* image);
	~GMGLTexture();

public:
	void init();

public:
	virtual void beginTexture() override;
	virtual void endTexture() override;

private:
	AutoPtr<Image> m_image;
	GLuint m_id;
};

END_NS
#endif