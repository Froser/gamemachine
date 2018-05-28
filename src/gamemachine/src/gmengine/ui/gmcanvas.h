#ifndef __GMCANVAS_H__
#define __GMCANVAS_H__
#include <gmcommon.h>
#include "gmcontrols.h"
BEGIN_NS

class GMControl;
class GMGameObject;
class GMModel;
class GMTextGameObject;
class GMSprite2DGameObject;
class GMSystemEvent;
class GMCanvas;

struct GMCanvasControlArea
{
	enum Area
	{
		ButtonArea,
		ButtonFillArea,
	};
};

struct GMCanvasTextureInfo
{
	ITexture* texture = nullptr;
	GMint width = 0;
	GMint height = 0;
};

GM_PRIVATE_OBJECT(GMCanvasResourceManager)
{
	GMTextGameObject* textObject = nullptr;
	GMSprite2DGameObject* spriteObject = nullptr;
	Vector<GMCanvasTextureInfo> textureCache;
	Vector<GMCanvas*> canvases;
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
	const GMCanvasTextureInfo& getTexture(GMsize_t index);
	GMsize_t addTexture(ITexture* texture, GMint width, GMint height);

	//! 注册一个画布到资源管理器。
	/*!
	  注册进来的画布为一个环状链表，用于切换焦点。<BR>
	  例如，用户切换到下一个或者前一个焦点时，通过此环状画布链表，使相应的画布的某一个控件获得焦点。
	  \param canvas 待注册的画布。
	*/
	void registerCanvas(GMCanvas* canvas);

	void onRenderRectResized();

	inline const Vector<GMCanvas*>& getCanvases()
	{
		D(d);
		return d->canvases;
	}

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

	inline GMTextGameObject* getTextObject()
	{
		D(d);
		return d->textObject;
	}

	inline GMSprite2DGameObject* getSpriteObject()
	{
		D(d);
		return d->spriteObject;
	}

	GMModel* getScreenQuadModel();
	GMGameObject* getScreenQuad();
};

GM_PRIVATE_OBJECT(GMCanvas)
{
	GMCanvasResourceManager* manager = nullptr;
	GMCanvas* nextCanvas; // 下一个画布默认为自己
	GMCanvas* prevCanvas; // 上一个画布默认为自己
	Vector<GMControl*> controls;
	GMControl* focusControl = nullptr;
	Vector<GMStyleHolder*> defaultstyles;
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
	HashMap<GMCanvasControlArea::Area, GMRect> areas;
};

class GMCanvas : public GMObject
{
	DECLARE_PRIVATE(GMCanvas)

public:
	GMCanvas(GMCanvasResourceManager* manager);
	~GMCanvas();

public:
	virtual void init();

public:
	void addArea(GMCanvasControlArea::Area area, const GMRect& rc);
	bool msgProc(GMSystemEvent* event);
	void render(GMfloat elpasedTime);
	void setNextCanvas(GMCanvas* nextCanvas);
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

	void addButton(
		GMint id,
		const GMString& text,
		GMint x,
		GMint y,
		GMint width,
		GMint height,
		bool isDefault,
		OUT GMControlButton** out
	);

	void drawText(
		const GMString& text,
		GMStyle* style,
		const GMRect& rc,
		bool bShadow = false,
		GMint nCount = -1,
		bool bCenter = false
	);

	void drawSprite(
		GMStyle* style,
		const GMRect& rc,
		GMfloat depth
	);

	void requestFocus(GMControl* control);

protected:
	virtual void initDefaultStyles();

private:
	void setDefaultStyle(GMControlType type, GMuint index, GMStyle* style);
	bool initControl(GMControl* control);
	void setPrevCanvas(GMCanvas* prevCanvas);
	void refresh();
	void focusDefaultControl();
	void removeAllControls();
	GMControl* getControlAtPoint(const GMPoint& pt);
	bool onCycleFocus(bool goForward);
	void onMouseMove(const GMPoint& pt);

public:
	inline GMCanvas* getNextCanvas()
	{
		D(d);
		return d->nextCanvas;
	}

	inline GMCanvas* getPrevCanvas()
	{
		D(d);
		return d->prevCanvas;
	}

	inline const Vector<GMControl*>& getControls()
	{
		D(d);
		return d->controls;
	}

	inline bool canKeyboardInput()
	{
		D(d);
		return d->keyboardInput;
	}

	inline void setKeyboardInput(bool keyboardInput)
	{
		D(d);
		d->keyboardInput = keyboardInput;
	}

public:
	static void clearFocus();

	// 一些全局的状态
	static GMfloat s_timeRefresh;
	static GMControl* s_controlFocus;
	static GMControl* s_controlPressed;
};

END_NS
#endif