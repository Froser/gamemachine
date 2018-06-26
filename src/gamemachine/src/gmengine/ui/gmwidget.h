#ifndef __GMCANVAS_H__
#define __GMCANVAS_H__
#include <gmcommon.h>
BEGIN_NS

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
struct ITypoEngine;

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
		BorderArea,
		TextEditBorderArea,
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
	GMSprite2DGameObject* opaqueSpriteObject = nullptr;
	GMBorder2DGameObject* borderObject = nullptr;
	Vector<GMWidget*> widgets;
	GMint backBufferWidth = 0;
	GMint backBufferHeight = 0;
	Map<GMint, GMCanvasTextureInfo> textureResources;
	ITexture* whiteTexture = nullptr;
};

class GMWidgetResourceManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMWidgetResourceManager)

public:
	enum TextureType
	{
		WhiteTexture,
		Skin,
		Border,
		UserDefine,
	};

public:
	GMWidgetResourceManager(const IRenderContext* context);
	~GMWidgetResourceManager();

public:
	ITypoEngine* getTypoEngine();
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

	inline GMSprite2DGameObject* getOpaqueSpriteObject()
	{
		D(d);
		return d->opaqueSpriteObject;
	}

	inline GMBorder2DGameObject* getBorderObject()
	{
		D(d);
		return d->borderObject;
	}
};

struct GMShadowStyle
{
	bool hasShadow = false;
	GMint offsetX = 1;
	GMint offsetY = 1;
	GMVec4 color = GMVec4(0, 0, 0, 1);
};

GM_PRIVATE_OBJECT(GMStyle)
{
	GMWidgetResourceManager::TextureType texture;
	GMFontHandle font = 0;
	GMRect rc;
	GMElementBlendColor textureColor;
	GMElementBlendColor fontColor;
	GMShadowStyle shadowStyle;
};

class GMStyle : public GMObject
{
	GM_DECLARE_PRIVATE(GMStyle)
	GM_ALLOW_COPY_DATA(GMStyle)

public:
	GMStyle() = default;

public:
	void setTexture(GMWidgetResourceManager::TextureType texture, const GMRect& rc, const GMVec4& defaultTextureColor = GMVec4(1, 1, 1, 1));
	void setFont(GMFontHandle font);
	void setFontColor(const GMVec4& defaultColor = GMVec4(1, 1, 1, 1));
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

	inline GMWidgetResourceManager::TextureType getTexture()
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

GM_PRIVATE_OBJECT(GMWidget)
{
	GMWidgetResourceManager* manager = nullptr;
	IWindow* parentWindow = nullptr;
	GMWidget* nextWidget; // 下一个Widget默认为自己
	GMWidget* prevWidget; // 上一个Widget默认为自己
	Vector<GMControl*> controls;
	GMControl* focusControl = nullptr;
	GMControlBorder* borderControl = nullptr;
	GMint borderMarginLeft = 10;
	GMint borderMarginTop = 30;
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
	GMPoint titleOffset;
	GMStyle titleStyle;
	GMStyle shadowStyle;

	GMStyle whiteTextureStyle;

	GMFloat4 colorTopLeft = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorTopRight = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorBottomLeft = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorBottomRight = GMFloat4(0, 0, 0, 0);

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
	GM_DECLARE_PRIVATE(GMWidget)

public:
	GMWidget(GMWidgetResourceManager* manager);
	~GMWidget();

public:
	void addArea(GMTextureArea::Area area, const GMRect& rc);
	void render(GMfloat elpasedTime);
	void setNextWidget(GMWidget* nextWidget);
	void addControl(GMControl* control);
	const GMRect& getArea(GMTextureArea::Area area);

	GMStyle getTitleStyle();
	void setTitleStyle(const GMStyle& titleStyle);

	void setTitleVisible(
		bool visible
	);

	void setTitle(
		const GMString& text,
		const GMPoint& offset = { 10, 0 }
	);

	void addLabel(
		const GMString& text,
		const GMVec4& fontColor,
		GMint x,
		GMint y,
		GMint width,
		GMint height,
		bool isDefault,
		OUT GMControlLabel** out
	);

	void addButton(
		const GMString& text,
		GMint x,
		GMint y,
		GMint width,
		GMint height,
		bool isDefault,
		OUT GMControlButton** out
	);

	void addBorder(
		const GMRect& corner,
		const GMint marginLeft = 10,
		const GMint marginTop = 30
	);

	void addTextEdit(
		const GMString& text,
		GMint x,
		GMint y,
		GMint width,
		GMint height,
		bool isDefault,
		const GMRect& cornerRect,
		OUT GMControlTextEdit** out
	);

	void addTextArea(
		const GMString& text,
		GMint x,
		GMint y,
		GMint width,
		GMint height,
		bool isDefault,
		const GMRect& cornerRect,
		OUT GMControlTextArea** out
	);

	void drawText(
		const GMString& text,
		GMStyle& style,
		const GMRect& rc,
		bool shadow = false,
		bool center = false,
		bool newLine = true
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

	void requestFocus(GMControl* control);
	void setSize(GMint width, GMint height);

public:
	virtual bool msgProc(GMSystemEvent* event);
	virtual void onInit() {}
	virtual bool onTitleMouseDown(const GMSystemMouseEvent* event);
	virtual bool onTitleMouseMove(const GMSystemMouseEvent* event);
	virtual bool onTitleMouseUp(const GMSystemMouseEvent* event);
	virtual void onRenderTitle();
	virtual void onUpdateSize();

protected:
	void addBorder(
		GMint x,
		GMint y,
		GMint width,
		GMint height,
		const GMRect& cornerRect,
		OUT GMControlBorder** out
	);

private:
	bool initControl(GMControl* control);
	void setPrevCanvas(GMWidget* prevCanvas);
	void refresh();
	void focusDefaultControl();
	void removeAllControls();
	GMControl* getControlAtPoint(const GMPoint& pt);
	bool onCycleFocus(bool goForward);
	void onMouseMove(const GMPoint& pt);
	void mapRect(GMRect& rc);
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

	inline void setPosition(GMint x, GMint y)
	{
		D(d);
		d->x = x;
		d->y = y;
	}

	inline void setBorderMargin(GMint left, GMint top)
	{
		D(d);
		d->borderMarginLeft = left;
		d->borderMarginTop = top;
	}

	inline GMint getTitleHeight()
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

public:
	static void clearFocus(GMWidget* sender);

	// 一些全局的状态
	static GMfloat s_timeRefresh;
	static GMControl* s_controlFocus;
	static GMControl* s_controlPressed;
};

END_NS
#endif