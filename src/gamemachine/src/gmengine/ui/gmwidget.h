#ifndef __GMCANVAS_H__
#define __GMCANVAS_H__
#include <gmcommon.h>
BEGIN_NS

class GMControl;
class GMGameObject;
class GMModel;
class GMTextGameObject;
class GMSprite2DGameObject;
class GMSystemEvent;
class GMSystemMouseEvent;
class GMWidget;
class GMControlStatic;
class GMControlButton;

struct GMControlState
{
	enum State
	{
		Normal,
		Disabled,
		Hidden,
		Focus,
		MouseOver,
		Pressed,

		EndOfControlState,
	};
};

GM_PRIVATE_OBJECT(GMElementBlendColor)
{
	GMVec4 states[GMControlState::EndOfControlState];
	GMVec4 current;
};

class GMElementBlendColor : public GMObject
{
	DECLARE_PRIVATE(GMElementBlendColor)
	GM_ALLOW_COPY_DATA(GMElementBlendColor)

public:
	GMElementBlendColor() = default;

public:
	void init(const GMVec4& defaultColor, const GMVec4& disabledColor = GMVec4(.5f, .5f, .5f, .78f), const GMVec4& hiddenColor = GMVec4(0));
	void blend(GMControlState::State state, GMfloat elapsedTime, GMfloat rate = .7f);

public:
	inline const GMVec4& getCurrent()
	{
		D(d);
		return d->current;
	}

	inline void setCurrent(const GMVec4& current)
	{
		D(d);
		d->current = current;
	}

	inline GMVec4* getStates()
	{
		D(d);
		return d->states;
	}
};

struct GMTextureArea
{
	enum Area
	{
		CaptionArea,
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
	const IRenderContext* context = nullptr;
	GMTextGameObject* textObject = nullptr;
	GMSprite2DGameObject* spriteObject = nullptr;
	Vector<GMWidget*> widgets;
	GMint backBufferWidth = 0;
	GMint backBufferHeight = 0;
	GMGameObject* screenQuad = nullptr;
	GMModel* screenQuadModel = nullptr;
	Map<GMint, GMCanvasTextureInfo> textureResources;
};

class GMWidgetResourceManager : public GMObject
{
	DECLARE_PRIVATE(GMWidgetResourceManager)

public:
	enum TextureType
	{
		Skin,
		UserDefine,
	};

public:
	GMWidgetResourceManager(const IRenderContext* context);
	~GMWidgetResourceManager();

public:
	const GMCanvasTextureInfo& getTexture(TextureType type);
	void addTexture(TextureType type, ITexture* texture, GMint width, GMint height);

	virtual const IRenderContext* getContext()
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

GM_PRIVATE_OBJECT(GMStyle)
{
	GMWidgetResourceManager::TextureType texture;
	GMuint font = 0;
	GMRect rc;
	GMElementBlendColor textureColor;
	GMElementBlendColor fontColor;
};

class GMStyle : public GMObject
{
	DECLARE_PRIVATE(GMStyle)
	GM_ALLOW_COPY_DATA(GMStyle)

public:
	GMStyle() = default;

public:
	void setTexture(GMWidgetResourceManager::TextureType texture, const GMRect& rc, const GMVec4& defaultTextureColor = GMVec4(1, 1, 1, 1));
	void setFont(GMuint font, const GMVec4& defaultColor = GMVec4(1, 1, 1, 1));
	void setFontColor(GMControlState::State state, const GMVec4& color);
	void setTextureColor(GMControlState::State state, const GMVec4& color);
	void refresh();

public:
	inline GMElementBlendColor& getTextureColor()
	{
		D(d);
		return d->textureColor;
	}

	inline GMElementBlendColor& getFontColor()
	{
		D(d);
		return d->fontColor;
	}

	inline const GMRect& getTextureRect()
	{
		D(d);
		return d->rc;
	}

	inline GMWidgetResourceManager::TextureType getTexture()
	{
		D(d);
		return d->texture;
	}
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
	bool title = false;
	GMint titleHeight = 20;
	GMString titleText;
	GMStyle titleStyle;

	GMfloat colorTopLeft[3] = { 0 };
	GMfloat colorTopRight[3] = { 0 };
	GMfloat colorBottomLeft[3] = { 0 };
	GMfloat colorBottomRight[3] = { 0 };
	GMint width = 0;
	GMint height = 0;
	GMint x = 0;
	GMint y = 0;
	HashMap<GMTextureArea::Area, GMRect> areas;

	bool movingWidget = false;
	GMPoint movingStartPt;
};

class GMWidget : public GMObject
{
	DECLARE_PRIVATE(GMWidget)

public:
	GMWidget(GMWidgetResourceManager* manager);
	~GMWidget();

public:
	void init();
	void addArea(GMTextureArea::Area area, const GMRect& rc);
	void render(GMfloat elpasedTime);
	void setNextCanvas(GMWidget* nextCanvas);
	void addControl(GMControl* control);
	const GMRect& getArea(GMTextureArea::Area area);

	void setTitleVisible(
		bool visible
	);

	void setTitle(
		const GMString& text
	);

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

public:
	virtual bool msgProc(GMSystemEvent* event);
	virtual bool onTitleMouseDown(const GMSystemMouseEvent* event);
	virtual bool onTitleMouseMove(const GMSystemMouseEvent* event);
	virtual bool onTitleMouseUp(const GMSystemMouseEvent* event);
	virtual void onRenderTitle();

private:
	bool initControl(GMControl* control);
	void setPrevCanvas(GMWidget* prevCanvas);
	void refresh();
	void focusDefaultControl();
	void removeAllControls();
	GMControl* getControlAtPoint(const GMPoint& pt);
	bool onCycleFocus(bool goForward);
	void onMouseMove(const GMPoint& pt);
	void adjustRect(GMRect& rc);

protected:
	void initStyles();

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

	inline void setSize(GMint width, GMint height)
	{
		D(d);
		d->width = width;
		d->height = height;
	}

	inline void setPosition(GMint x, GMint y)
	{
		D(d);
		d->x = x;
		d->y = y;
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