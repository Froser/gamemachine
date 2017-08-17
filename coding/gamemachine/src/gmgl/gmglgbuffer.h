#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include "common.h"
BEGIN_NS

// GBuffer 类型，对应pass着色器layout的顺序
enum class GBufferTextureType
{
	Position,
	Normal,
	AmbientTexture,
	DiffuseTexture,
	Tangent,
	Bitangent,
	NormalMap,
	EndOfTextureType,
};

GM_PRIVATE_OBJECT(GMGLGBuffer)
{
	GLuint fbo = 0;
	GLuint textures[(GMint)GBufferTextureType::EndOfTextureType] = { 0 };
	GLuint depthTexture = 0;
	GMuint windowWidth = 0;
	GMuint windowHeight = 0;
};

class GMGLGBuffer : public GMObject
{
	DECLARE_PRIVATE(GMGLGBuffer)

public:
	GMGLGBuffer();
	~GMGLGBuffer();

public:
	void dispose();
	bool init(GMuint windowWidth, GMuint windowHeight);
	void bindForWriting();
	void bindForReading();
	void releaseBind();
	void setReadBuffer(GBufferTextureType TextureType);

public:
	inline const GMuint& getWidth() { D(d); return d->windowWidth; }
	inline const GMuint& getHeight() { D(d); return d->windowHeight; }
};

END_NS
#endif