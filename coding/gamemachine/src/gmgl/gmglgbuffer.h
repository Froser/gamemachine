#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include "common.h"
BEGIN_NS

enum class GBufferTextureType
{
	Position,
	Diffuse,
	Normal,
	Texcoord,
	EndOfTextureType,
};

GM_PRIVATE_OBJECT(GMGLGBuffer)
{
	GLuint fbo = 0;
	GLuint textures[(GMint)GBufferTextureType::EndOfTextureType] = { 0 };
	GLuint depthTexture = 0;
};

class GMGLGBuffer : public GMObject
{
	DECLARE_PRIVATE(GMGLGBuffer)

public:
	GMGLGBuffer();
	~GMGLGBuffer();

public:
	void dispose();
	bool init(GMuint WindowWidth, GMuint WindowHeight);
	void bindForWriting();
	void bindForReading();
	void setReadBuffer(GBufferTextureType TextureType);
};

END_NS
#endif