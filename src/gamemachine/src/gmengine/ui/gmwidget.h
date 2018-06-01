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
class GMWidget;

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

GM_PRIVATE_OBJECT(GMWidgetResourceManager)
{
	const GMContext* context = nullptr;
	GMTextGameObject* textObject = nullptr;
	GMSprite2DGameObject* spriteObject = nullptr;
	Vector<GMCanvasTextureInfo> textureCache;
	Vector<GMWidget*> widgets;
	GMint backBufferWidth = 0;
	GMint backBufferHeight = 0;
	GMGameObject* screenQuad = nullptr;
	GMModel* screenQuadModel = nullptr;
};

class GMWidgetResourceManager : public GMObject, public IContext
{
	DECLARE_PRIVATE(GMWidgetResourceManager)

public:
	GMWidgetResourceManager(const GMContext* context);
	~GMWidgetResourceManager();

public:
	const GMCanvasTextureInfo& getTexture(GMsize_t index);
	GMsize_t addTexture(ITexture* texture, GMint width, GMint height);

	virtual const GMContext* getContext()
	{
		D(d);
		return d->context;
	}

	//! 注册一个画布到资源管理器。
	/*!
	  注册进来的画布为一个环状链表，用于切换焦点。<BR>
	  例如，用户切换到下一个或者前一个焦点时，通过此环状画布链表，使相应的画布的某一个控件获得焦点。
	  \param widget 待注册的画布。
	*/
	void registerWidget(GMWidget* widget);

	void onRenderRectResized();

	inline const Vector<GMWidget*>& getCanvases()
	{
		D(d);
		return d->widgets;
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

GM_PRIVATE_OBJECT(GMWidget)
{
	GMWidgetResourceManager* manager = nullptr;
	IWindow* parentWindow = nullptr;
	GMWidget* nextCanvas; // 下一个画布默认为自己
	GMWidget* prevCanvas; // 上一个画布默认为自己
	Vector<GMControl*> controls;
	GMControl* focusControl = nullptr;
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

class GMWidget : public GMObject
{
	DECLARE_PRIVATE(GMWidget)

public:
	GMWidget(GMWidgetResourceManager* manager);
	~GMWidget();

public:
	virtual void init() {}

public:
	void addArea(GMCanvasControlArea::Area area, const GMRect& rc);
	bool msgProc(GMSystemEvent* event);
	void render(GMfloat elpasedTime);
	void setNextCanvas(GMWidget* nextCanvas);
	void addControl(GMControl* control);
	const GMRect& getArea(GMCanvasControlArea::Area area);

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
		GMStyle& style,
		const GMRect& rc,
		bool shadow = false,
		bool center = false
	);

	void drawSprite(
		GMStyle& style,
		const GMRect& rc,
		GMfloat depth
	);

	void requestFocus(GMControl* control);

private:
	bool initControl(GMControl* control);
	void setPrevCanvas(GMWidget* prevCanvas);
	void refresh();
	void focusDefaultControl();
	void removeAllControls();
	GMControl* getControlAtPoint(const GMPoint& pt);
	bool onCycleFocus(bool goForward);
	void onMouseMove(const GMPoint& pt);

public:
	inline IWindow* getParentWindow()
	{
		D(d);
		return d->parentWindow;
	}

	inline void setParentWindow(IWindow* window)
	{
		D(d);
		d->parentWindow = window;
	}

	inline void setMinimize(bool minimize)
	{
		D(d);
		d->minimized = minimize;
	}

	inline bool getMinimize()
	{
		D(d);
		return d->minimized;
	}

	inline void setVisible(bool visible)
	{
		D(d);
		d->visible = visible;
	}

	inline bool getVisible()
	{
		D(d);
		return d->visible;
	}

	inline GMWidget* getNextCanvas()
	{
		D(d);
		return d->nextCanvas;
	}

	inline GMWidget* getPrevCanvas()
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