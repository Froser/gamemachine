#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include <gmcommon.h>
#include <gmgbuffer.h>
BEGIN_NS

// GBuffer 类型，对应pass着色器layout的顺序
enum class GBufferGeometryType
{
	// 顶点属性
	Position,
	Normal,
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
	Ks_Shininess,
	HasNormalMap_Refractivity,
	EndOfMaterialType,
};

// 当前渲染的状态，渲染到哪一步了
enum class GMGLDeferredRenderState
{
	PassingGeometry, //正在进行普通渲染或geometry pass
	PassingMaterial, //正在传递材质

	PassingLight, // 光照计算
	EndOfRenderState = PassingLight
};

constexpr GMint GMGLGBuffer_TotalTurn = (GMint) GMGLDeferredRenderState::EndOfRenderState;
class GMGraphicEngine;
GM_PRIVATE_OBJECT(GMGLGBufferDep)
{
	GMuint fbo[GMGLGBuffer_TotalTurn] = { 0 };
	GMuint textures[(GMint)GBufferGeometryType::EndOfGeometryType] = { 0 };
	GMuint materials[(GMint)GBufferMaterialType::EndOfMaterialType] = { 0 };
	GMuint depthBuffers[(GMint)GMGLDeferredRenderState::EndOfRenderState] = { 0 };
	GMint renderWidth = 0;
	GMint renderHeight = 0;
	GMint currentTurn = 0;
	GMRect renderRect = { 0 };
	GMRect viewport = { 0 };
};

class GMGLShaderProgram;
class GMGLGBufferDep : public GMObject
{
	DECLARE_PRIVATE(GMGLGBufferDep)

public:
	GMGLGBufferDep() = default;
	~GMGLGBufferDep();

public:
	void adjustViewport();
	void beginPass();
	bool nextPass();
	void dispose();
	bool init(const GMRect& renderRect);
	void bindForWriting();
	void bindForReading();
	void releaseBind();
	void setReadBuffer(GBufferGeometryType textureType);
	void setReadBuffer(GBufferMaterialType materialType);
	void newFrame();
	void activateTextures();
	void copyDepthBuffer(GMuint target);

public:
	inline const GMint& getWidth() { D(d); return d->renderWidth; }
	inline const GMint& getHeight() { D(d); return d->renderHeight; }

private:
	bool createFrameBuffers(GMGLDeferredRenderState state, GMint textureCount, GMuint* textureArray);
	bool drawBuffers(GMuint count);
};

class GMGLGBuffer : public GMGBuffer
{
protected:
	virtual IFramebuffers* createGeometryFramebuffers() override;
	virtual IFramebuffers* createMaterialFramebuffers() override;

public:
	virtual void geometryPass(GMGameObject *objects[], GMuint count) override;
	virtual void lightPass() override;
};


END_NS
#endif