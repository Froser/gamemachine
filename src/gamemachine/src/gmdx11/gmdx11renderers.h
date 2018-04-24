#ifndef __GMDX11RENDERERS_H__
#define __GMDX11RENDERERS_H__
#include <gmcommon.h>
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
BEGIN_NS

class GMDx11Renderer_CubeMap;
class GMModel;
struct GMDx11CubeMapState
{
	bool hasCubeMap = false;
	GMDx11Renderer_CubeMap* cubeMapRenderer = nullptr;
	GMModel* model = nullptr;
};

GM_PRIVATE_OBJECT(GMDx11Renderer)
{
	GMComPtr<ID3D11InputLayout> inputLayout;
	GMComPtr<ID3DX11Effect> effect;
	ID3D11DeviceContext* deviceContext = nullptr;
	ID3DX11EffectTechnique* technique = nullptr;
	ID3DX11EffectRasterizerVariable* rasterizer = nullptr;
	ID3DX11EffectBlendVariable* blend = nullptr;
	ID3DX11EffectDepthStencilVariable* depthStencil = nullptr;
	const GMShaderVariablesDesc* variablesDesc = nullptr;
};

class GMDx11Renderer : public GMObject, public IRenderer
{
	DECLARE_PRIVATE(GMDx11Renderer)

public:
	GMDx11Renderer();

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void draw(GMModel* model) override;
	virtual const char* getTechniqueName() = 0;

protected:
	inline GMDx11GraphicEngine* getEngine()
	{
		return gm_static_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
	}

	inline const GMShaderVariablesDesc* getVariablesDesc()
	{
		D(d);
		if (!d->variablesDesc)
		{
			d->variablesDesc = &getEngine()->getShaderProgram()->getDesc();
		}
		return d->variablesDesc;
	}

protected:
	virtual void prepareTextures(GMModel* model);
	virtual void setTextures(GMModel* model);
	virtual void passAllAndDraw(GMModel* model);
	virtual void prepareBuffer(GMModel* model);
	virtual void prepareLights();
	virtual void prepareMaterials(GMModel* model);
	virtual void prepareRasterizer(GMModel* model);
	virtual void prepareBlend(GMModel* model);
	virtual void prepareDepthStencil(GMModel* model);
	virtual void applyTextureAttribute(GMModel* model, ITexture* texture, GMTextureType type, GMint index);

	ID3DX11EffectTechnique* getTechnique();
	ITexture* GMDx11Renderer::getTexture(GMTextureFrames& frames);

public:
	static GMDx11CubeMapState& getCubeMapState();
};

class GMDx11Renderer_3D : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_3D";
	}
};

class GMDx11Renderer_2D : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_2D";
	}
};

class GMDx11Renderer_Glyph : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Glyph";
	}
};

class GMDx11Renderer_CubeMap : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_CubeMap";
	}

public:
	virtual void prepareTextures(GMModel* model);
	virtual void setTextures(GMModel* model);
};

class GMDx11Renderer_Filter : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Filter";
	}

	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void draw(GMModel* model) override;
};

class GMDx11Renderer_Deferred_3D: public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Deferred_3D";
	}

	virtual void passAllAndDraw(GMModel* model);
};

class GMDx11Renderer_Deferred_3D_LightPass : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Deferred_3D_LightPass";
	}

	virtual void prepareMaterials(GMModel* model) override {}
	virtual void prepareLights() override {}

private:
	void passAllAndDraw(GMModel* model);
	void setDeferredTexturesBeforeApply();
};

END_NS
#endif