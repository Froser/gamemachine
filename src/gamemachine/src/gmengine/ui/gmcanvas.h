#ifndef __GMCANVAS_H__
#define __GMCANVAS_H__
#include <gmcommon.h>
#include "gmcontrols.h"
BEGIN_NS

class GMControl;
class GMGameObject;
class GMModel;

GM_PRIVATE_OBJECT(GMCanvasResourceManager)
{
	Vector<ITexture*> textureCache;
	GMint backBufferWidth = 0;
	GMint backBufferHeight = 0;
	GMGameObject* screenQuad = nullptr;
	GMModel* screenQuadModel = nullptr;
};

class GMCanvasResourceManager : public GMObject
{
	DECLARE_PRIVATE(GMCanvasResourceManager)

public:
	GMCanvasResourceManager();
	~GMCanvasResourceManager();

public:
	ITexture* getTexture(size_t);
	void addTexture(ITexture*);

	void onRenderRectResized();

	inline GMint getBackBufferWidth()
	{
		D(d);
		return d->backBufferWidth;
	}

	inline GMint getBackBufferHeight()
	{
		D(d);
		return d->backBufferHeight;
	}

	GMModel* getScreenQuadModel();
	GMGameObject* getScreenQuad();
};

GM_PRIVATE_OBJECT(GMCanvas)
{
	GMCanvasResourceManager* manager = nullptr;
	Vector<GMControl*> controls;
	GMControl* focusControl = nullptr;
	Vector<GMElementHolder> defaultElements;
	GMfloat timeLastRefresh = 0;
	GMControl* controlMouseOver = nullptr;
	bool nonUserEvents = false;
	bool keyboardInput = false;
	bool mouseInput = false;
	bool visible = true;
	bool minimized = false;
	bool caption = false;
	GMfloat colorTopLeft[3] = { 0 };
	GMfloat colorTopRight[3] = { 0 };
	GMfloat colorBottomLeft[3] = { 0 };
	GMfloat colorBottomRight[3] = { 0 };
	GMint width = 0;
	GMint height = 0;
	GMint x = 0;
	GMint y = 0;
};

class GMCanvas : public GMObject
{
	DECLARE_PRIVATE(GMCanvas)

public:
	GMCanvas(GMCanvasResourceManager* manager);

public:
	virtual void onRender() = 0;

public:
	void render(GMfloat elpasedTime);
	void addControl(GMControl* control);

	void addStatic(
		GMint id,
		const GMString& text,
		GMint x,
		GMint y,
		GMint width,
		GMint height,
		bool isDefault,
		OUT GMControlStatic** out
	);

	void drawText(
		const GMString& text,
		GMElement* element,
		const GMRect& rcDest,
		bool bShadow = false,
		GMint nCount = -1,
		bool bCenter = false
	);

private:
	void initDefaultElements();
	bool initControl(GMControl* control);
	void refresh();
	void focusDefaultControl();

public:
	static void clearFocus();

	// 一些全局的状态
	static GMfloat s_timeRefresh;
	static GMControl* s_controlFocus;
	static GMControl* s_controlPressed;
};

END_NS
#endif