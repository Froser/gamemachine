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
	GMStencilOptions stencilOptions;
	Vector<ILight*> lights;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
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
	virtual void draw(const List<GMGameObject*>& forwardRenderingObjects, const List<GMGameObject*>& deferredRenderingObjects) override;
	virtual void addLight(AUTORELEASE ILight* light) override;
	virtual void removeLights() override;
	virtual void setStencilOptions(const GMStencilOptions& options) override;
	virtual const GMStencilOptions& getStencilOptions() override;
	virtual void setShaderLoadCallback(IShaderLoadCallback* cb) override;

public:
	const GMFilterMode::Mode getCurrentFilterMode();
	void draw(const List<GMGameObject*>& objects);

protected:
	void createFilterFramebuffer();

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

	inline IShaderLoadCallback* getShaderLoadCallback()
	{
		D(d);
		return d->shaderLoadCallback;
	}

	inline const Vector<ILight*>& getLights()
	{
		D(d);
		return d->lights;
	}

private:
	IGBuffer* createGBuffer();

public:
	static constexpr const GMuint getMaxLightCount()
	{
		return 50;
	}
};

END_NS
#endif