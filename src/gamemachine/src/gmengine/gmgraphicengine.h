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
};

class GMGraphicEngine : public GMObject, public IGraphicEngine
{
	DECLARE_PRIVATE(GMGraphicEngine)

public:
	~GMGraphicEngine();

public:
	GMFramebuffersStack& getFramebuffersStack()
	{
		D(d);
		return d->framebufferStack;
	}

protected:
	void createFilterFramebuffer();

	inline GMGameObject* getFilterQuad()
	{
		D(d);
		return d->filterQuad;
	}

	inline IFramebuffers* getFilterFramebuffers()
	{
		D(d);
		return d->filterFramebuffers;
	}
};

END_NS
#endif