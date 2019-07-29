#ifndef __GMGLRENDERERS_H__
#define __GMGLRENDERERS_H__
#include <gmcommon.h>
#include <gmenums.h>
#include <gmmodel.h>
#include "gmglgraphic_engine.h"
BEGIN_NS

class GMGLTechnique;
class GMGLShadowFramebuffers;
class GMGammaHelper
{
public:
	void setGamma(GMGLTechnique* tech, GMGraphicEngine* engine, IShaderProgram* shaderProgram);

private:
	GMfloat m_gamma = 0.f;
};

GM_PRIVATE_CLASS(GMGLTechnique);
class GMGLTechnique : public GMObject, public ITechnique
{
	GM_DECLARE_PRIVATE(GMGLTechnique)
	GM_DISABLE_COPY_ASSIGN(GMGLTechnique)
	GM_FRIEND_CLASS(GMGammaHelper)

public:
	GMGLTechnique(const IRenderContext* context);
	~GMGLTechnique();

	virtual void draw(GMModel* model) override;
	virtual IShaderProgram* getShaderProgram() = 0;

protected:
	virtual void beforeDraw(GMModel* model) = 0;
	virtual void afterDraw(GMModel* model) = 0;
	virtual void beginScene(GMScene* scene) override;
	virtual void endScene() override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;

protected:
	virtual void activateTextureTransform(GMModel* model, GMTextureType type);
	virtual GMint32 activateTexture(GMModel* model, GMTextureType type);
	virtual void deactivateTexture(GMTextureType type);
	virtual GMint32 getTextureID(GMTextureType type);
	virtual bool drawTexture(GMModel* model, GMTextureType type);
	virtual GMTextureAsset getTexture(GMTextureSampler& frames);
	virtual void updateCameraMatrices(IShaderProgram* shaderProgram);
	virtual void setCascadeEndClip(GMCascadeLevel level, GMfloat endClip);
	virtual void setCascadeCameraVPMatrices(GMCascadeLevel level);
	virtual void initShadow();

protected:
	void prepareScreenInfo(IShaderProgram* shaderProgram);
	void prepareShaderAttributes(GMModel* model);
	void prepareBlend(GMModel* model);
	void prepareFrontFace(GMModel* model);
	void prepareCull(GMModel* model);
	void prepareDepth(GMModel* model);
	void prepareDebug(GMModel* model);
	void prepareLights();
	void prepareShadow(const GMShadowSourceDesc* shadowSourceDesc, GMGLShadowFramebuffers* shadowFramebuffers, bool hasShadow);
	GMIlluminationModel prepareIlluminationModel(GMModel* model);
	void updateBoneTransforms(IShaderProgram* shaderProgram, GMModel* model);
	void updateNodeTransforms(IShaderProgram* shaderProgram, GMModel* model);

private:
	void startDraw(GMModel* model);
};

GM_PRIVATE_CLASS(GMGLTechnique_3D);
class GMGLTechnique_3D : public GMGLTechnique
{
	GM_DECLARE_PRIVATE(GMGLTechnique_3D)
	GM_DECLARE_BASE(GMGLTechnique)

public:
	GMGLTechnique_3D(const IRenderContext* context);
	~GMGLTechnique_3D();

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual IShaderProgram* getShaderProgram() override;

protected:
	void prepareMaterial(const GMShader& shader);
	void prepareTextures(GMModel* model, GMIlluminationModel illuminationModel);
	void drawDebug();

private:
	GMTextureAsset getWhiteTexture();
};

class GMGLTechnique_2D : public GMGLTechnique_3D
{
public:
	using GMGLTechnique_3D::GMGLTechnique_3D;

public:
	virtual void beforeDraw(GMModel* model) override;
	virtual void draw(GMModel* model) override;
	virtual void initShadow() {}

protected:
	void prepareTextures(GMModel* model);
};

class GMGLTechnique_CubeMap : public GMGLTechnique_3D
{
public:
	using GMGLTechnique_3D::GMGLTechnique_3D;

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void initShadow() {}

protected:
	void prepareTextures(GMModel* model);
};

GM_PRIVATE_CLASS(GMGLTechnique_Filter);
class GMGLTechnique_Filter : public GMGLTechnique
{
	GM_DECLARE_PRIVATE(GMGLTechnique_Filter)
	GM_DECLARE_BASE(GMGLTechnique)

public:
	GMGLTechnique_Filter(const IRenderContext* context);
	~GMGLTechnique_Filter();

private:
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual IShaderProgram* getShaderProgram() override;
	virtual void initShadow() {}

protected:
	virtual GMint32 activateTexture(GMModel* model, GMTextureType type);

protected:
	void prepareTextures(GMModel* model);

private:
	void setHDR(IShaderProgram* shaderProgram);
};

GM_PRIVATE_CLASS(GMGLTechnique_LightPass);
class GMGLTechnique_LightPass : public GMGLTechnique
{
	GM_DECLARE_PRIVATE(GMGLTechnique_LightPass)
	GM_DECLARE_BASE(GMGLTechnique)

public:
	GMGLTechnique_LightPass(const IRenderContext* context);
	~GMGLTechnique_LightPass();

protected:
	virtual IShaderProgram* getShaderProgram() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;

protected:
	void prepareLights();
	void prepareTextures(GMModel* model);
};

class GMGLTechnique_3D_Shadow : public GMGLTechnique_3D
{
	typedef GMGLTechnique_3D Base;

public:
	using GMGLTechnique_3D::GMGLTechnique_3D;

protected:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void initShadow() {}
};

class GMGLTechnique_Particle : public GMGLTechnique_2D
{
public:
	using GMGLTechnique_2D::GMGLTechnique_2D;
};

class GMGLTechnique_Custom : public GMGLTechnique_3D
{
public:
	using GMGLTechnique_3D::GMGLTechnique_3D;

	virtual IShaderProgram* getShaderProgram() override;
};

END_NS
#endif