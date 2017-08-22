#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include "common.h"
BEGIN_NS

// GBuffer 类型，对应pass着色器layout的顺序
enum class GBufferTextureType
{
	// 顶点属性
	Position,
	Normal,
	AmbientTexture,
	DiffuseTexture,
	Tangent,
	Bitangent,
	NormalMap,
	EndOfTextureType,
};

enum class GBufferMaterialType
{
	// 材质属性
	Ka,
	Kd,
	Ks,
	EndOfMaterialType,
};

enum
{
	GMGLGBuffer_Rendering,
	GMGLGBuffer_MaterialPass,

	GMGLGBuffer_TotalTurn,
};

GM_PRIVATE_OBJECT(GMGLGBuffer)
{
	GLuint fbo[GMGLGBuffer_TotalTurn] = { 0 };
	GLuint textures[(GMint)GBufferTextureType::EndOfTextureType] = { 0 };
	GLuint materials[(GMint)GBufferMaterialType::EndOfMaterialType] = { 0 };
	GLuint depthTexture = 0;
	GMuint windowWidth = 0;
	GMuint windowHeight = 0;
	GMint currentTurn = 0;
};

class GMGLShaderProgram;
class GMGLGBuffer : public GMObject
{
	DECLARE_PRIVATE(GMGLGBuffer)

public:
	GMGLGBuffer();
	~GMGLGBuffer();

public:
	void beginPass();
	bool nextPass();
	void dispose();
	bool init(GMuint windowWidth, GMuint windowHeight);
	void bindForWriting();
	void bindForReading();
	void releaseBind();
	void setReadBuffer(GBufferTextureType textureType);
	void setReadBuffer(GBufferMaterialType materialType);
	void newFrame();
	void activateTextures(GMGLShaderProgram* shaderProgram);
	void activateMaterials(GMGLShaderProgram* shaderProgram);

public:
	inline const GMuint& getWidth() { D(d); return d->windowWidth; }
	inline const GMuint& getHeight() { D(d); return d->windowHeight; }

private:
	bool drawBuffers(GMuint count);
};

END_NS
#endif