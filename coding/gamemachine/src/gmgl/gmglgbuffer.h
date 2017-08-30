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
	HasNormalMap,
	EndOfMaterialType,
};

// 当前渲染的状态，渲染到哪一步了
enum class GMGLDeferredRenderState
{
	PassingGeometry, //正在进行普通渲染或geometry pass
	PassingMaterial, //正在传递材质
	EndOfRenderState,
};

constexpr GMint GMGLGBuffer_TotalTurn = (GMint) GMGLDeferredRenderState::EndOfRenderState;

GM_PRIVATE_OBJECT(GMGLGBuffer)
{
	GLuint fbo[GMGLGBuffer_TotalTurn] = { 0 };
	GLuint textures[(GMint)GBufferGeometryType::EndOfGeometryType] = { 0 };
	GLuint materials[(GMint)GBufferMaterialType::EndOfMaterialType] = { 0 };
	GLuint depthBuffers[(GMint)GMGLDeferredRenderState::EndOfRenderState] = { 0 };
	GMuint windowWidth = 0;
	GMuint windowHeight = 0;
	GMint currentTurn = 0;
};

class GMGLShaderProgram;
class GMGLGBuffer : public GMObject
{
	DECLARE_PRIVATE(GMGLGBuffer)

public:
	GMGLGBuffer() = default;
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
	void copyDepthBuffer(GLuint target);

public:
	inline const GMuint& getWidth() { D(d); return d->windowWidth; }
	inline const GMuint& getHeight() { D(d); return d->windowHeight; }

private:
	bool createFrameBuffers(GMGLDeferredRenderState state, GMint textureCount, GLuint* textureArray);
	bool drawBuffers(GMuint count);
};

GM_PRIVATE_OBJECT(GMGLFramebuffer)
{
	GLuint fbo = 0;
	GLuint quadVAO = 0;
	GLuint quadVBO = 0;
	GLuint texture = 0;
	GLuint depthBuffer = 0;
	GMuint windowWidth = 0;
	GMuint windowHeight = 0;
	GMuint effects = (GMuint) GMEffects::None;
	bool hasBegun = false;
};

class GMGLFramebuffer : public GMObject
{
	DECLARE_PRIVATE(GMGLFramebuffer)

public:
	GMGLFramebuffer() = default;
	~GMGLFramebuffer();

public:
	void dispose();
	bool init(GMuint windowWidth, GMuint windowHeight);
	void beginDrawEffects(GMEffects effects);
	void endDrawEffects();
	void draw(GMGLShaderProgram* program);

public:
	inline GLuint framebuffer() { D(d); if (d->effects) return d->fbo; return 0; }
	inline bool hasBegun() { D(d); return d->hasBegun; }

private:
	void bindForWriting();
	void bindForReading();
	void releaseBind();
	void newFrame();
	void createQuad();
	void renderQuad();
	void disposeQuad();
	void useShaderProgramAndApplyEffects(GMGLShaderProgram* program);
};

END_NS
#endif