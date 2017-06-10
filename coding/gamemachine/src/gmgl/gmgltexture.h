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

	const char* operator [](TextureIndex t);

private:
	std::map<TextureIndex, std::string> m_uniformNames;
};

inline std::string getTextureUniformName(TextureIndex t)
{
	static GMGLTextureShaderNames s;
	return s[t];
}

class GMGLTexture : public ITexture
{
public:
	GMGLTexture(AUTORELEASE GMImage* image);
	~GMGLTexture();

public:
	void init();

public:
	virtual void drawTexture(TextureFrames* frames) override;

private:
	bool m_inited;
	AutoPtr<GMImage> m_image;
	GLuint m_id;
};

END_NS
#endif