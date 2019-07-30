#ifndef __GMDX11TECHNIQUES_H__
#define __GMDX11TECHNIQUES_H__
#include <gmcommon.h>
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
BEGIN_NS

class GMDx11Technique_CubeMap;
class GMModel;
class GMDx11EffectVariableBank;
struct GMDx11CubeMapState;
struct GMDx11RasterizerStates;
struct GMDx11BlendStates;
struct GMDx11DepthStencilStates;

struct GMTextureAttributeBank
{
	ID3DX11EffectScalarVariable* enabled = nullptr;
	ID3DX11EffectScalarVariable* offsetX = nullptr;
	ID3DX11EffectScalarVariable* offsetY = nullptr;
	ID3DX11EffectScalarVariable* scaleX = nullptr;
	ID3DX11EffectScalarVariable* scaleY = nullptr;
};

GM_PRIVATE_CLASS(GMDx11Technique);
class GM_EXPORT GMDx11Technique : public GMObject, public ITechnique
{
	GM_DECLARE_PRIVATE(GMDx11Technique)
	GM_DISABLE_COPY_ASSIGN(GMDx11Technique)

public:
	GMDx11Technique(const IRenderContext* context);
	~GMDx11Technique();

public:
	virtual void beginScene(GMScene* scene) override;
	virtual void endScene() override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void draw(GMModel* model) override;
	virtual const char* getTechniqueName() = 0;
	virtual const IRenderContext* getContext();

public:
	ID3DX11Effect* getEffect();

protected:
	virtual void prepareScreenInfo();
	virtual void prepareTextures(GMModel* model);
	virtual void passAllAndDraw(GMModel* model);
	virtual void prepareBuffer(GMModel* model);
	virtual void prepareLights();
	virtual void prepareMaterials(GMModel* model);
	virtual void prepareRasterizer(GMModel* model);
	virtual void prepareBlend(GMModel* model);
	virtual void prepareDepthStencil(GMModel* model);
	virtual void prepareDebug(GMModel* model);
	virtual void applyTextureAttribute(GMModel* model, GMTextureAsset texture, GMTextureType type);
	virtual void initShadow();
	virtual void prepareShadow(bool isDrawingShadow);
	virtual ID3DX11EffectTechnique* getTechnique();
	GMTextureAsset getTexture(GMTextureSampler& sampler);
	void setGamma(IShaderProgram* shaderProgram);
	GMDx11EffectVariableBank& getVarBank();

private:
	GMTextureAsset getWhiteTexture();

protected:
	GMDx11GraphicEngine* getEngine();
	GMModel* getCurrentModel();
	void updateBoneTransforms(IShaderProgram* shaderProgram, GMModel* model);
	void updateNodeTransforms(IShaderProgram* shaderProgram, GMModel* model);
	virtual void setCascadeEndClip(GMCascadeLevel level, GMfloat endClip);
	virtual void setCascadeCameraVPMatrices(GMCascadeLevel level);

public:
	static const std::string& getTechniqueNameByTechniqueId(GMRenderTechniqueID id);
};

class GMDx11Technique_3D : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_3D";
	}
};

class GMDx11Technique_2D : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_2D";
	}

	virtual void prepareTextures(GMModel* model) override;
	virtual void draw(GMModel* model) override;
	virtual void initShadow() {}
};

class GMDx11Technique_Text : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Text";
	}

	virtual void initShadow() {}
};

class GMDx11Technique_CubeMap : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_CubeMap";
	}

public:
	virtual void prepareTextures(GMModel* model);
	virtual void initShadow() {}
};

GM_PRIVATE_CLASS(GMDx11Technique_Filter);
class GMDx11Technique_Filter : public GMDx11Technique
{
	GM_DECLARE_PRIVATE(GMDx11Technique_Filter)
	GM_DECLARE_BASE(GMDx11Technique)

public:
	GMDx11Technique_Filter(const IRenderContext* context);

private:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Filter";
	}

	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void draw(GMModel* model) override;
	virtual void passAllAndDraw(GMModel* model) override;
	virtual void initShadow() {}

	void setHDR(IShaderProgram* shaderProgram);
};

class GMDx11Technique_Deferred_3D: public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Deferred_3D";
	}

	virtual void passAllAndDraw(GMModel* model);
	virtual void initShadow() {}
};

class GMDx11Technique_Deferred_3D_LightPass : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Deferred_3D_LightPass";
	}

	virtual void prepareMaterials(GMModel* model) override {}
	virtual void prepareTextures(GMModel* model) override;

private:
	void passAllAndDraw(GMModel* model);
};

class GMDx11Technique_3D_Shadow : public GMDx11Technique_3D
{
public:
	using GMDx11Technique_3D::GMDx11Technique_3D;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_3D_Shadow";
	}

	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void draw(GMModel* model) override;
	virtual void initShadow() {}
};

class GMDx11Technique_Particle : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Particle";
	}
};

GM_PRIVATE_CLASS(GMDx11Technique_Custom);
class GMDx11Technique_Custom : public GMDx11Technique
{
	GM_DECLARE_PRIVATE(GMDx11Technique_Custom)
	GM_DECLARE_BASE(GMDx11Technique)

public:
	GMDx11Technique_Custom(const IRenderContext* context);
	~GMDx11Technique_Custom();

protected:
	virtual ID3DX11EffectTechnique* getTechnique() override;

protected:
	virtual const char* getTechniqueName() override;
};

END_NS
#endif