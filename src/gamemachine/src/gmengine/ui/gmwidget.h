#ifndef __GMWIDGET_H__
#define __GMWIDGET_H__
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
class GMUIConfiguration;
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

GM_PRIVATE_CLASS(GMElementBlendColor);
class GMElementBlendColor
{
	GM_DECLARE_PRIVATE(GMElementBlendColor)

public:
	GMElementBlendColor();
	~GMElementBlendColor();
	GMElementBlendColor(const GMElementBlendColor&);
	GMElementBlendColor(GMElementBlendColor&&) GM_NOEXCEPT;
	GMElementBlendColor& operator=(const GMElementBlendColor&);
	GMElementBlendColor& operator=(GMElementBlendColor&&) GM_NOEXCEPT;

public:
	void init(const GMVec4& defaultColor, const GMVec4& disabledColor = GMVec4(.5f, .5f, .5f, .78f), const GMVec4& hiddenColor = GMVec4(0));
	void blend(GMControlState::State state, GMfloat elapsedTime, GMfloat rate = .7f);
	const GMVec4& getCurrent();
	void setCurrent(const GMVec4& current);
	GMVec4* getStates();
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

GM_PRIVATE_CLASS(GMWidgetResourceManager);
class GM_EXPORT GMWidgetResourceManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMWidgetResourceManager)

public:
	GMWidgetResourceManager(const IRenderContext* context);
	~GMWidgetResourceManager();

public:
	GMWidget* createWidget();

public:
	ITypoEngine* getTypoEngine();
	const GMCanvasTextureInfo& getTexture(GMlong id);
	GMlong addTexture(GMAsset texture, GMint32 width, GMint32 height);
	const IRenderContext* getContext();

	//! 注册一个画布到资源管理器。
	/*!
	  注册进来的画布为一个环状链表，用于切换焦点。<BR>
	  例如，用户切换到下一个或者前一个焦点时，通过此环状画布链表，使相应的画布的某一个控件获得焦点。
	  \param widget 待注册的画布。
	*/
	void registerWidget(GMWidget* widget);
	void onRenderRectResized();
	void setUIConfiguration(const GMUIConfiguration& config);
	const Vector<GMWidget*>& getCanvases();
	GMint32 getBackBufferWidth();
	GMint32 getBackBufferHeight();
	GMTextGameObject* getTextObject();
	GMSprite2DGameObject* getSpriteObject();
	GMSprite2DGameObject* getOpaqueSpriteObject();
	GMBorder2DGameObject* getBorderObject();
	GMlong getWhiteTextureId() GM_NOEXCEPT;
};

struct GMShadowStyle
{
	bool hasShadow = false;
	GMint32 offsetX = 1;
	GMint32 offsetY = 1;
	GMVec4 color = GMVec4(0, 0, 0, 1);
};

GM_PRIVATE_CLASS(GMStyle);
class GM_EXPORT GMStyle
{
	GM_DECLARE_PRIVATE(GMStyle)
	GM_DECLARE_PROPERTY(GMRect, CornerRc)

public:
	GMStyle(
		const GMVec4& defaultTextureColor = GMVec4(1, 1, 1, 1),
		const GMVec4& disabledTextureColor = GMVec4(.5f, .5f, .5f, .78f),
		const GMVec4& hiddenTextureColor = GMVec4(0)
	);
	~GMStyle();
	GMStyle(const GMStyle&);
	GMStyle(GMStyle&&) GM_NOEXCEPT;
	GMStyle& operator=(const GMStyle& rhs);
	GMStyle& operator=(GMStyle&& rhs) GM_NOEXCEPT;

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
	GMElementBlendColor& getTextureColor();
	GMElementBlendColor& getFontColor();
	GMFontHandle getFont();
	const GMRect& getTextureRect();
	GMlong getTexture();
	void setShadowStyle(const GMShadowStyle& shadowStyle);
	const GMShadowStyle& getShadowStyle();
};

struct GMWidgetTextureArea
{
	GMlong textureId;
	GMRect rc;
	GMRect cornerRc;
};

enum class GMOverflowStyle
{
	Auto,
	Hidden,
	Visible,
	Scroll,
};

//! 表示控件在容器中的位置标识。
enum class GMControlPositionFlag
{
	Auto, //!< 控件在容器中的位置是浮动的，这意味着会受到容器滚动条的影响。
	Fixed, //!< 控件在容器中的位置是固定的。
};

GM_PRIVATE_CLASS(GMWidget);
class GM_EXPORT GMWidget : public GMObject
{
	GM_FRIEND_CLASS(GMWidgetResourceManager);

	GM_DECLARE_PRIVATE(GMWidget)
	GM_DECLARE_PROPERTY(bool, Minimum)
	GM_DECLARE_PROPERTY(bool, Visible)
	GM_DECLARE_PROPERTY(GMint32, ScrollStep)
	GM_DECLARE_PROPERTY(GMint32, ScrollOffsetY)
	GM_DECLARE_PROPERTY(GMint32, VerticalScrollbarWidth)

public:
	enum OverflowFlag
	{
		CannotScroll = 0,
		CanScrollUp = 1,
		CanScrollDown = 2,
	};

private:
	GMWidget(GMWidgetResourceManager* manager);

public:
	~GMWidget();

public:
	void initStyles();
	void addArea(GMTextureArea::Area area, GMlong textureId, const GMRect& rc, const GMRect& cornerRc);
	void render(GMfloat elpasedTime);
	void setNextWidget(GMWidget* nextWidget);
	void addControl(GMControl* control);
	const GMWidgetTextureArea& getArea(GMTextureArea::Area area);

	IWindow* getParentWindow() GM_NOEXCEPT;
	void setParentWindow(IWindow* window) GM_NOEXCEPT;
	GMWidget* getNextCanvas() GM_NOEXCEPT;
	GMWidget* getPrevCanvas() GM_NOEXCEPT;
	const Vector<GMControl*>& getControls() GM_NOEXCEPT;
	bool canKeyboardInput() GM_NOEXCEPT;
	void setKeyboardInput(bool keyboardInput) GM_NOEXCEPT;
	void setPosition(GMint32 x, GMint32 y) GM_NOEXCEPT;
	void setBorderMargin(GMint32 left, GMint32 top) GM_NOEXCEPT;
	GMint32 getTitleHeight() GM_NOEXCEPT;
	GMWidgetResourceManager* getManager() GM_NOEXCEPT;
	GMRect getSize() GM_NOEXCEPT;
	void resetControlMouseOver() GM_NOEXCEPT;
	GMOverflowStyle getOverflow() GM_NOEXCEPT;
	void setOverflow(GMOverflowStyle);
	GMint32 getContentPaddingLeft() GM_NOEXCEPT;
	void setContentPaddingLeft(GMint32);
	GMint32 getContentPaddingTop() GM_NOEXCEPT;
	void setContentPaddingTop(GMint32);
	GMint32 getContentPaddingRight() GM_NOEXCEPT;
	void setContentPaddingRight(GMint32);
	GMint32 getContentPaddingBottom() GM_NOEXCEPT;
	void setContentPaddingBottom(GMint32);

	GMStyle getTitleStyle();
	void setTitleStyle(const GMStyle& titleStyle);

	void setTitleVisible(
		bool visible
	);

	void setTitle(
		const GMString& text,
		const GMPoint& offset = { 10, 0 }
	);

	void drawText(
		GMControlPositionFlag positionFlag,
		const GMString& text,
		GMStyle& style,
		const GMRect& rc,
		bool shadow = false,
		bool center = false,
		bool newLine = true,
		GMint32 lineSpacing = 0
	);

	void drawText(
		GMControlPositionFlag positionFlag,
		GMTypoTextBuffer* textBuffer,
		GMStyle& style,
		const GMRect& rc,
		bool shadow = false
	);

	void drawSprite(
		GMControlPositionFlag positionFlag,
		GMStyle& style,
		const GMRect& rc,
		GMfloat depth
	);

	void drawRect(
		GMControlPositionFlag positionFlag,
		const GMVec4& bkColor,
		const GMRect& rc,
		bool isOpaque,
		GMfloat depth
	);

	void drawBorder(
		GMControlPositionFlag positionFlag,
		GMStyle& style,
		const GMRect& rc,
		GMfloat depth
	);

	void drawStencil(
		GMControlPositionFlag positionFlag,
		GMRect rc,
		GMfloat depth,
		bool drawRc,
		const GMVec4& color = GMVec4(1, 1, 1, 1)
	);

	void useStencil(
		bool inside
	);

	void endStencil();

	void requestFocus(GMControl* control);
	void setSize(GMint32 width, GMint32 height);
	bool verticalScroll(GMint32 offset);
	bool verticalScrollTo(GMint32 value);

public:
	virtual bool handleSystemEvent(GMSystemEvent* event);
	virtual void onInit() {}
	virtual bool onTitleMouseDown(const GMSystemMouseEvent* event);
	virtual bool onTitleMouseMove(const GMSystemMouseEvent* event);
	virtual bool onTitleMouseUp(const GMSystemMouseEvent* event);
	virtual bool onMouseWheel(const GMSystemMouseEvent* event);
	virtual void onRenderTitle();
	virtual void onUpdateSize();

protected:
	void addBorder(
		const GMRect& corner,
		const GMint32 marginLeft = 10,
		const GMint32 marginTop = 30
	);

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
	GMControl* getControlAtPoint(const GMPoint& pt);
	bool onCycleFocus(bool goForward);
	void onMouseMove(const GMPoint& pt);
	void mapRect(GMControlPositionFlag positionFlag, GMRect& rc);
	GMRect getContentRect();
	GMint32 getContentOverflowFlag();
	void createVerticalScrollbar();
	void updateVerticalScrollbar();
	void disableVerticalScrollbar();
	bool needShowVerticalScrollbar();
	GMSystemMouseEvent* adjustMouseEvent(GMSystemMouseEvent* event, const GMControl* control);
	void calculateControlBoundingRect();
	void updateScaling();

public:
	static void clearFocus(GMWidget* sender);

	// 一些全局的状态
	static GMfloat s_timeRefresh;
	static GMControl* s_controlFocus;
	static GMControl* s_controlPressed;
};

END_NS
#endif