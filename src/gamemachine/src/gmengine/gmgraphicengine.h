#ifndef __GMGRAPHICENGINE_H__
#define __GMGRAPHICENGINE_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <gmmodel.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMFramebuffersStack)
{
	Stack<IFramebuffers*> framebuffers;
};

class GMFramebuffersStack : public GMObject
{
	DECLARE_PRIVATE(GMFramebuffersStack);

public:
	void push(IFramebuffers* framebuffers);
	IFramebuffers* pop();
	IFramebuffers* peek();
};

GM_PRIVATE_OBJECT(GMGraphicEngine)
{
	IFramebuffers* filterFramebuffers = nullptr;
	GMGameObject* filterQuad = nullptr;
	GMScopePtr<GMModel> filterQuadModel;
	GMFramebuffersStack framebufferStack;
	IGBuffer* gBuffer = nullptr;
	GMRenderConfig renderConfig;
	GMDebugConfig debugConfig;
};

class GMGraphicEngine : public GMObject, public IGraphicEngine
{
	DECLARE_PRIVATE(GMGraphicEngine)

public:
	GMGraphicEngine();
	~GMGraphicEngine();

public:
	virtual IGBuffer* getGBuffer() override;
	virtual IFramebuffers* getFilterFramebuffers() override;
	virtual void draw(
		GMGameObject *forwardRenderingObjects[],
		GMuint forwardRenderingCount,
		GMGameObject *deferredRenderingObjects[],
		GMuint deferredRenderingCount) override;

public:
	const GMFilterMode::Mode getCurrentFilterMode();

protected:
	void createFilterFramebuffer();
	void draw(GMGameObject *objects[], GMuint count);

public:
	inline GMFramebuffersStack& getFramebuffersStack()
	{
		D(d);
		return d->framebufferStack;
	}

protected:
	inline GMGameObject* getFilterQuad()
	{
		D(d);
		return d->filterQuad;
	}

private:
	IGBuffer* createGBuffer();
};

END_NS
#endif