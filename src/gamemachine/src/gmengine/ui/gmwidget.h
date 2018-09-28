#ifndef __GMCANVAS_H__
#define __GMCANVAS_H__
#include <gmcommon.h>
#include <gmassets.h>
BEGIN_NS

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

#define GM_WHEEL_DELTA WHEEL_DELTA

class GMControl;
class GMGameObject;
class GMModel;
class GMTextGameObject;
class GMSprite2DGameObject;
class GMBorder2DGameObject;
class GMSystemEvent;
class GMSystemMouseEvent;
class GMWidget;
class GMControlLabel;
class GMControlButton;
class GMControlBorder;
class GMControlTextEdit;
class GMControlTextArea;
class GMTypoTextBuffer;
class GMControlScrollBar;
struct ITypoEngine;
struct GMWidgetTextureArea;

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
	GM_DECLARE_PRIVATE(GMElementBlendColor)
	GM_ALLOW_COPY_MOVE(GMElementBlendColor)

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
		BorderArea,
		ScrollBarUp,
		ScrollBarDown,
		ScrollBarThumb,
		ScrollBarTrack,
		TextEditBorderArea,
	};
};

struct GMCanvasTextureInfo
{
	GMAsset texture;
	GMint32 width = 0;
	GMint32 height = 0;
};

GM_PRIVATE_OBJECT(GMWidgetResourceManager)
{
	const IRenderContext* context = nullptr;
	GMOwnedPtr<GMTextGameObject> textObject;
	GMOwnedPtr<GMSprite2DGameObject> spriteObject;
	GMOwnedPtr<GMSprite2DGameObject> opaqueSpriteObject;
	GMOwnedPtr<GMBorder2DGameObject> borderObject;
	Vector<GMWidget*> widgets;
	GMint32 backBufferWidth = 0;
	GMint32 backBufferHeight = 0;
	Vector<GMCanvasTextureInfo> textureResources;
	GMTextureAsset whiteTexture;
	GMAtomic<GMlong> textureId;
	GMlong whiteTextureId = 0;
};

class GMWidgetResourceManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMWidgetResourceManager)

public:
	GMWidgetResourceManager(const IRenderContext* context);
	~GMWidgetResourceManager() = default;

public:
	ITypoEngine* getTypoEngine();
	const GMCanvasTextureInfo& getTexture(GMlong id);
	GMlong addTexture(GMAsset texture, GMint32 width, GMint32 height);

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

	inline GMint32 getBackBufferWidth()
	{
		D(d);
		return d->backBufferWidth;
	}

	inline GMint32 getBackBufferHeight()
	{
		D(d);
		return d->backBufferHeight;
	}

	inline GMTextGameObject* getTextObject()
	{
		D(d);
		return d->textObject.get();
	}

	inline GMSprite2DGameObject* getSpriteObject()
	{
		D(d);
		return d->spriteObject.get();
	}

	inline GMSprite2DGameObject* getOpaqueSpriteObject()
	{
		D(d);
		return d->opaqueSpriteObject.get();
	}

	inline GMBorder2DGameObject* getBorderObject()
	{
		D(d);
		return d->borderObject.get();
	}

	inline GMlong getWhiteTextureId() GM_NOEXCEPT
	{
		D(d);
		return d->whiteTextureId;
	}
};

struct GMShadowStyle
{
	bool hasShadow = false;
	GMint32 offsetX = 1;
	GMint32 offsetY = 1;
	GMVec4 color = GMVec4(0, 0, 0, 1);
};

GM_PRIVATE_OBJECT(GMStyle)
{
	GMlong texture;
	GMFontHandle font = 0;
	GMRect rc;
	GMElementBlendColor textureColor;
	GMElementBlendColor fontColor;
	GMShadowStyle shadowStyle;
};

class GMStyle : public GMObject
{
	GM_DECLARE_PRIVATE(GMStyle)
	GM_ALLOW_COPY_MOVE(GMStyle)

public:
	GMStyle(
		const GMVec4& defaultTextureColor = GMVec4(1, 1, 1, 1),
		const GMVec4& disabledTextureColor = GMVec4(.5f, .5f, .5f, .78f),
		const GMVec4& hiddenTextureColor = GMVec4(0)
	);

public:
	void setTexture(const GMWidgetTextureArea& idRc);
	void setFont(GMFontHandle font);
	void setFontColor(const GMVec4& defaultColor = GMVec4(1, 1, 1, 1));
	void setFontColor(GMControlState::State state, const GMVec4& color);
	void resetTextureColor(
		const GMVec4& defaultTextureColor = GMVec4(1, 1, 1, 1),
		const GMVec4& disabledColor = GMVec4(.5f, .5f, .5f, .78f),
		const GMVec4& hiddenColor = GMVec4(0)
	);
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

	inline GMFontHandle getFont()
	{
		D(d);
		return d->font;
	}

	inline const GMRect& getTextureRect()
	{
		D(d);
		return d->rc;
	}

	inline GMlong getTexture()
	{
		D(d);
		return d->texture;
	}

	inline void setShadowStyle(const GMShadowStyle& shadowStyle)
	{
		D(d);
		d->shadowStyle = shadowStyle;
	}

	inline const GMShadowStyle& getShadowStyle()
	{
		D(d);
		return d->shadowStyle;
	}
};

struct GMWidgetTextureArea
{
	GMlong textureId;
	GMRect rc;
};

enum class GMOverflowStyle
{
	Auto,
	Hidden,
	Visible,
	Scroll,
};

GM_PRIVATE_OBJECT(GMWidget)
{
	GMWidgetResourceManager* manager = nullptr;
	IWindow* parentWindow = nullptr;
	GMWidget* nextWidget; // 下一个Widget默认为自己
	GMWidget* prevWidget; // 上一个Widget默认为自己
	GM_OWNED Vector<GMControl*> controls;
	GMControl* focusControl = nullptr;
	GMControlBorder* borderControl = nullptr;
	GMint32 borderMarginLeft = 10;
	GMint32 borderMarginTop = 30;
	GMint32 contentPaddingLeft = 0;
	GMint32 contentPaddingTop = 0;
	GMint32 contentPaddingRight = 0;
	GMint32 contentPaddingBottom = 0;
	GMfloat timeLastRefresh = 0;
	GMControl* controlMouseOver = nullptr;
	bool nonUserEvents = false;
	bool keyboardInput = false;
	bool mouseInput = false;
	bool visible = true;
	bool minimized = false;
	bool title = false;
	GMint32 titleHeight = 20;
	GMString titleText;
	GMPoint titleOffset;
	GMStyle titleStyle;
	GMStyle shadowStyle;

	GMStyle whiteTextureStyle;
	GMOverflowStyle overflow = GMOverflowStyle::Auto;

	GMFloat4 colorTopLeft = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorTopRight = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorBottomLeft = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorBottomRight = GMFloat4(0, 0, 0, 0);

	GMint32 width = 0;
	GMint32 height = 0;
	GMint32 x = 0;
	GMint32 y = 0;
	HashMap<GMTextureArea::Area, GMWidgetTextureArea> areas;

	bool movingWidget = false;
	GMPoint movingStartPt;

	GMint32 scrollOffsetY = 0;
	GMint32 scrollStep = 10;
	GMRect controlBoundingBox = { 0 };
};

class GMWidget : public GMObject
{
	GM_DECLARE_PRIVATE(GMWidget)
	GM_DECLARE_PROPERTY(Minimum, minimized, bool)
	GM_DECLARE_PROPERTY(Visible, visible, bool)
	GM_DECLARE_PROPERTY(Overflow, overflow, GMOverflowStyle)
	GM_DECLARE_PROPERTY(ContentPaddingLeft, contentPaddingLeft, GMint32)
	GM_DECLARE_PROPERTY(ContentPaddingTop, contentPaddingTop, GMint32)
	GM_DECLARE_PROPERTY(ContentPaddingRight, contentPaddingRight, GMint32)
	GM_DECLARE_PROPERTY(ContentPaddingBottom, contentPaddingBottom, GMint32)
	GM_DECLARE_PROPERTY(ScrollStep, scrollStep, GMint32)
	GM_DECLARE_PROPERTY(ScrollOffsetY, scrollOffsetY, GMint32)

public:
	GMWidget(GMWidgetResourceManager* manager);
	~GMWidget();

public:
	void addArea(GMTextureArea::Area area, GMlong textureId, const GMRect& rc);
	void render(GMfloat elpasedTime);
	void setNextWidget(GMWidget* nextWidget);
	void addControl(GMControl* control);
	const GMWidgetTextureArea& getArea(GMTextureArea::Area area);

	GMStyle getTitleStyle();
	void setTitleStyle(const GMStyle& titleStyle);

	void setTitleVisible(
		bool visible
	);

	void setTitle(
		const GMString& text,
		const GMPoint& offset = { 10, 0 }
	);

	void addBorder(
		const GMRect& corner,
		const GMint32 marginLeft = 10,
		const GMint32 marginTop = 30
	);

public:
	void drawText(
		const GMString& text,
		GMStyle& style,
		const GMRect& rc,
		bool shadow = false,
		bool center = false,
		bool newLine = true,
		GMint32 lineSpacing = 0
	);

	void drawText(
		GMTypoTextBuffer* textBuffer,
		GMStyle& style,
		const GMRect& rc,
		bool shadow = false
	);

	void drawSprite(
		GMStyle& style,
		const GMRect& rc,
		GMfloat depth
	);

	void drawRect(
		const GMVec4& bkColor,
		const GMRect& rc,
		bool isOpaque,
		GMfloat depth
	);

	void drawBorder(
		GMStyle& style,
		const GMRect& cornerRc,
		const GMRect& rc,
		GMfloat depth
	);

	void drawStencil(
		const GMRect& rc,
		GMfloat depth,
		bool drawRc,
		const GMVec4& color = GMVec4(1, 1, 1, 1),
		bool clearCurrentStencil = true
	);

	void useStencil(
		bool inside
	);

	void endStencil();

	void requestFocus(GMControl* control);
	void setSize(GMint32 width, GMint32 height);
	
public:
	virtual bool msgProc(GMSystemEvent* event);
	virtual void onInit() {}
	virtual bool onTitleMouseDown(const GMSystemMouseEvent* event);
	virtual bool onTitleMouseMove(const GMSystemMouseEvent* event);
	virtual bool onTitleMouseUp(const GMSystemMouseEvent* event);
	virtual bool onMouseWheel(const GMSystemMouseEvent* event);
	virtual void onRenderTitle();
	virtual void onUpdateSize();

	//! 当控件大小发生变化时，调用此方法。
	/*!
	  此方法将会计算内部所有控件的边框大小的并集，作为控件是否超出内容矩形区域的依据。
	*/
	virtual void onControlRectChanged(GMControl* control);

protected:
	void addBorder(
		GMint32 x,
		GMint32 y,
		GMint32 width,
		GMint32 height,
		const GMRect& cornerRect,
		OUT GMControlBorder** out
	);

private:
	bool initControl(GMControl* control);
	void setPrevCanvas(GMWidget* prevCanvas);
	void refresh();
	void focusDefaultControl();
	void removeAllControls();
	GMControl* getControlAtPoint(GMPoint pt);
	bool onCycleFocus(bool goForward);
	void onMouseMove(const GMPoint& pt);
	void mapRect(GMRect& rc);
	void initStyles();
	GMRect getContentRect();

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

	inline GMWidget* getNextCanvas()
	{
		D(d);
		return d->nextWidget;
	}

	inline GMWidget* getPrevCanvas()
	{
		D(d);
		return d->prevWidget;
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

	inline void setPosition(GMint32 x, GMint32 y)
	{
		D(d);
		d->x = x;
		d->y = y;
	}

	inline void setBorderMargin(GMint32 left, GMint32 top)
	{
		D(d);
		d->borderMarginLeft = left;
		d->borderMarginTop = top;
	}

	inline GMint32 getTitleHeight()
	{
		D(d);
		return d->titleHeight;
	}

	inline GMWidgetResourceManager* getManager()
	{
		D(d);
		return d->manager;
	}

	inline GMRect getSize()
	{
		D(d);
		GMRect rc = { 0, 0, d->width, d->height };
		return rc;
	}

	inline void resetControlMouseOver()
	{
		D(d);
		d->controlMouseOver = nullptr;
	}

public:
	static void clearFocus(GMWidget* sender);

	// 一些全局的状态
	static GMfloat s_timeRefresh;
	static GMControl* s_controlFocus;
	static GMControl* s_controlPressed;
};

END_NS
#endif