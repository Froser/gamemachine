#ifndef __GMCANVAS_H__
#define __GMCANVAS_H__
#include <gmcommon.h>
#include "gmcontrols.h"
BEGIN_NS

class GMControl;

GM_PRIVATE_OBJECT(GMCanvasResourceManager)
{
	Vector<ITexture*> textureCache;
	GMint backBufferWidth = 0;
	GMint backBufferHeight = 0;
};

class GMCanvasResourceManager : public GMObject
{
	DECLARE_PRIVATE(GMCanvasResourceManager)

public:
	~GMCanvasResourceManager();

public:
	ITexture* getTexture(size_t);
	void addTexture(ITexture*);

	void onRenderRectResized();
};

GM_PRIVATE_OBJECT(GMCanvas)
{
	Vector<GMControl*> controls;
	Vector<GMElementHolder> defaultElements;
};

class GMCanvas : public GMObject
{
	DECLARE_PRIVATE(GMCanvas)

public:
	virtual void onRender() = 0;

public:
	void render();
	void addControl(GMControl* control);

private:
	bool initControl(GMControl* control);
};

END_NS
#endif