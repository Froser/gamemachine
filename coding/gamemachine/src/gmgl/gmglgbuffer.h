#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include "common.h"
BEGIN_NS

// GBuffer 类型，对应pass着色器layout的顺序
enum class GBufferGeometryType
{
	// 顶点属性
	Position,
	Normal_Eye,
	AmbientTexture,
	DiffuseTexture,
	Tangent_Eye,
	Bitangent_Eye,
	NormalMap,
	EndOfGeometryType,
};

enum class GBufferMaterialType
{
	// 材质属性
	Ka,
	Kd,
	Ks,
	Shininess,
	EndOfMaterialType,
};

enum class GBufferFlags
{
	// 标记
	HasNormalMap,
	EndOfFlags,
};

// 当前渲染的状态，渲染到哪一步了
enum class GMGLDeferredRenderState
{
	GeometryPass, //正在进行普通渲染或geometry pass
	PassingMaterial, //正在传递材质
	PassingFlags, //正在传递Flags
	EndOfRenderState,
};

constexpr GMint GMGLGBuffer_TotalTurn = (GMint) GMGLDeferredRenderState::EndOfRenderState;

GM_PRIVATE_OBJECT(GMGLGBuffer)
{
	GLuint fbo[GMGLGBuffer_TotalTurn] = { 0 };
	GLuint textures[(GMint)GBufferGeometryType::EndOfGeometryType] = { 0 };
	GLuint materials[(GMint)GBufferMaterialType::EndOfMaterialType] = { 0 };
	GLuint flags[(GMint)GBufferFlags::EndOfFlags] = { 0 };
	GLuint depthBuffer = 0;
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
	void setReadBuffer(GBufferGeometryType textureType);
	void setReadBuffer(GBufferMaterialType materialType);
	void newFrame();
	void activateTextures(GMGLShaderProgram* shaderProgram);
	void copyDepthBuffer();

public:
	inline const GMuint& getWidth() { D(d); return d->windowWidth; }
	inline const GMuint& getHeight() { D(d); return d->windowHeight; }

private:
	bool drawBuffers(GMuint count);
};

END_NS
#endif