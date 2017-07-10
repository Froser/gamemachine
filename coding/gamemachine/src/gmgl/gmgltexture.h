#ifndef __GMGL_TEXTURE_H__
#define __GMGL_TEXTURE_H__
#include "common.h"
#include "gmdatacore/image.h"
#include "foundation/utilities/utilities.h"
#include "gmdatacore/object.h"
#include <map>
#include <string>
BEGIN_NS

class GMGLTextureShaderNames
{
public:
	GMGLTextureShaderNames();

	GMString getName (GMTextureType type, GMint index = 0);

private:
	Map<GMTextureType, GMString> m_uniformNames;
};

inline GMString getTextureUniformName(GMTextureType t, GMint index)
{
	static GMGLTextureShaderNames s;
	return s.getName(t, index);
}

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