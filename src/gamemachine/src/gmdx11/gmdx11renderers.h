#ifndef __GMDX11RENDERERS_H__
#define __GMDX11RENDERERS_H__
#include <gmcommon.h>
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11Renderer)
{
	GMComPtr<ID3D11InputLayout> inputLayout;
	GMShader* shader = nullptr;
	ID3DX11EffectTechnique* mainTechnique = nullptr;
};

class GMDx11Renderer : public IRenderer
{
	DECLARE_PRIVATE(GMDx11Renderer)

public:
	GMDx11Renderer();

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void draw(IQueriable* painter, GMComponent* component, GMMesh* mesh) override;

private:
	inline GMDx11GraphicEngine* getEngine()
	{
		return gm_static_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
	}

private:
	void prepareBuffer(IQueriable* painter);
	void prepareRasterizer(GMComponent* component);
	void passAllAndDraw(GMComponent* component);
	ITexture* GMDx11Renderer::getTexture(GMTextureFrames& frames);
	void drawTextures();
};

END_NS
#endif