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
	GMint renderWidth = 0;
	GMint renderHeight = 0;
	GMint currentTurn = 0;
	GMRect clientRect = { 0 };
	GMRect viewport = { 0 };
};

class GMGLShaderProgram;
class GMGLGBuffer : public GMObject
{
	DECLARE_PRIVATE(GMGLGBuffer)

public:
	GMGLGBuffer() = default;
	~GMGLGBuffer();

public:
	void adjustViewport();
	void beginPass();
	bool nextPass();
	void dispose();
	bool init(const GMRect& clientRect);
	void bindForWriting();
	void bindForReading();
	void releaseBind();
	void setReadBuffer(GBufferGeometryType textureType);
	void setReadBuffer(GBufferMaterialType materialType);
	void newFrame();
	void activateTextures(GMGLShaderProgram* shaderProgram);
	void copyDepthBuffer(GLuint target);

public:
	inline const GMint& getWidth() { D(d); return d->renderWidth; }
	inline const GMint& getHeight() { D(d); return d->renderHeight; }

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
	GMint renderWidth = 0;
	GMint renderHeight = 0;
	GMuint effects = GMEffects::None;
	bool hasBegun = false;
	GMRect clientRect = { 0 };
	GMRect viewport = { 0 };
};

class GMGLFramebuffer : public GMObject
{
	DECLARE_PRIVATE(GMGLFramebuffer)

public:
	GMGLFramebuffer() = default;
	~GMGLFramebuffer();

public:
	void dispose();
	bool init(const GMRect& clientRect);
	void beginDrawEffects();
	void endDrawEffects();
	void draw(GMGLShaderProgram* program);
	GLuint framebuffer();

public:
	inline bool hasBegun() { D(d); return d->hasBegun; }

private:
	bool needRenderFramebuffer();
	void bindForWriting();
	void bindForReading();
	void releaseBind();
	void newFrame();
	void createQuad();
	void renderQuad();
	void disposeQuad();
	const char* useShaderProgramAndApplyEffect(GMGLShaderProgram* program, GMEffects effect);
};

END_NS
#endif