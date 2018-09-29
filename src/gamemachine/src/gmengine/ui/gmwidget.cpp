#include "stdafx.h"
#include "gmwidget.h"
#include "gmcontrols.h"
#include "foundation/gamemachine.h"
#include "../gameobjects/gmgameobject.h"
#include "../gameobjects/gm2dgameobject.h"
#include "../gmtypoengine.h"
#include "gmdata/gmmodel.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "gmengine/gmmessage.h"
#include "gmcontroltextedit.h"

namespace
{
	GMControl* getNextControl(GMControl* control)
	{
		GMWidget* parentWidget = control->getParent();
		GMuint32 index = control->getIndex() + 1;

		// 如果下一个控件不在此画布内，则跳到下一个画布进行查找
		while (index >= (GMuint32)parentWidget->getControls().size())
		{
			parentWidget = parentWidget->getNextCanvas();
			index = 0;
		}

		return parentWidget->getControls()[index];
	}

	GMControl* getPrevControl(GMControl* control)
	{
		GMWidget* parentCanvas = control->getParent();
		GMint32 index = gm_sizet_to_int(control->getIndex()) - 1;
		while (index < 0)
		{
			parentCanvas = parentCanvas->getPrevCanvas();
			if (!parentCanvas)
				parentCanvas = control->getParent();
			index = parentCanvas->getControls().size() - 1;
		}
		return parentCanvas->getControls()[index];
	}

	GMsize_t indexOf(const Vector<GMWidget*>& widgets, GMWidget* targetCanvas)
	{
		for (GMsize_t i = 0; i < widgets.size(); ++i)
		{
			if (widgets[i] == targetCanvas)
				return i;
		}
		return -1;
	}

	bool operator <(const GMCanvasTextureInfo& lhs, const GMCanvasTextureInfo& rhs)
	{
		return lhs.texture.getAsset() < rhs.texture.getAsset();
	}
}

class GMOpaqueSprite2DGameObject : public GMSprite2DGameObject
{
public:
	using GMSprite2DGameObject::GMSprite2DGameObject;

protected:
	virtual void initShader(GMShader& shader) override
	{
		GMSprite2DGameObject::initShader(shader);
		shader.setBlend(false);
	}
};

void GMElementBlendColor::init(const GMVec4& defaultColor, const GMVec4& disabledColor, const GMVec4& hiddenColor)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(state, GMControlState::Normal, GMControlState::EndOfControlState)
	{
		d->states[state] = defaultColor;
	};
	d->states[GMControlState::Disabled] = disabledColor;
	d->states[GMControlState::Hidden] = hiddenColor;
	d->current = hiddenColor;
}

void GMElementBlendColor::blend(GMControlState::State state, GMfloat elapsedTime, GMfloat rate)
{
	D(d);
	GMVec4 destColor = d->states[state];
	d->current = Lerp(d->current, destColor, 1 - Pow(rate, 60.f * elapsedTime));
}

GMStyle::GMStyle(
	const GMVec4& defaultTextureColor,
	const GMVec4& disabledTextureColor,
	const GMVec4& hiddenTextureColor
)
{
	D(d);
	resetTextureColor(defaultTextureColor, disabledTextureColor, hiddenTextureColor);
}

void GMStyle::setTexture(const GMWidgetTextureArea& idRc)
{
	D(d);
	d->texture = idRc.textureId;
	d->rc = idRc.rc;
}

void GMStyle::setFont(GMFontHandle font)
{
	D(d);
	d->font = font;
}

void GMStyle::setFontColor(const GMVec4& defaultColor)
{
	D(d);
	d->fontColor.init(defaultColor);
}

void GMStyle::setFontColor(GMControlState::State state, const GMVec4& color)
{
	D(d);
	d->fontColor.getStates()[state] = color;
}

void GMStyle::resetTextureColor(
	const GMVec4& defaultTextureColor,
	const GMVec4& disabledColor,
	const GMVec4& hiddenColor
)
{
	D(d);
	d->textureColor.init(defaultTextureColor, disabledColor, hiddenColor);
}

void GMStyle::setTextureColor(GMControlState::State state, const GMVec4& color)
{
	D(d);
	d->textureColor.getStates()[state] = color;
}

void GMStyle::refresh()
{
	D(d);
	d->textureColor.setCurrent(d->textureColor.getStates()[GMControlState::Hidden]);
	d->fontColor.setCurrent(d->fontColor.getStates()[GMControlState::Hidden]);
}

GMWidgetResourceManager::GMWidgetResourceManager(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->textureId = 0;
	d->textObject = gm_makeOwnedPtr<GMTextGameObject>(context->getWindow()->getRenderRect());
	d->textObject->setContext(context);

	d->spriteObject = gm_makeOwnedPtr<GMSprite2DGameObject>(context->getWindow()->getRenderRect());
	d->spriteObject->setContext(context);

	d->opaqueSpriteObject = gm_makeOwnedPtr<GMOpaqueSprite2DGameObject>(context->getWindow()->getRenderRect());
	d->opaqueSpriteObject->setContext(context);

	d->borderObject = gm_makeOwnedPtr<GMBorder2DGameObject>(context->getWindow()->getRenderRect());
	d->borderObject->setContext(context);

	addTexture(GMAsset(), 1, 1); //放入一个非法的Asset，占用textureId=0的情况

	GM.getFactory()->createWhiteTexture(context, d->whiteTexture);
	d->whiteTextureId = addTexture(d->whiteTexture, 1, 1);
}

GMlong GMWidgetResourceManager::addTexture(GMAsset texture, GMint32 width, GMint32 height)
{
	D(d);
	GMCanvasTextureInfo texInfo;
	texInfo.texture = texture;
	texInfo.width = width;
	texInfo.height = height;

	d->textureResources.push_back(std::move(texInfo));
	return d->textureId++;
}

void GMWidgetResourceManager::registerWidget(GMWidget* widget)
{
	D(d);
	for (auto c : d->widgets)
	{
		if (c == widget)
			return;
	}

	// 将widget设置成一个环
	d->widgets.push_back(widget);
	GMsize_t sz = d->widgets.size();
	if (sz > 1)
		d->widgets[sz - 2]->setNextWidget(widget);
	d->widgets[sz - 1]->setNextWidget(d->widgets[0]);
}

ITypoEngine* GMWidgetResourceManager::getTypoEngine()
{
	D(d);
	return d->textObject->getTypoEngine();
}

const GMCanvasTextureInfo& GMWidgetResourceManager::getTexture(GMlong id)
{
	D(d);
	return d->textureResources[id];
}

void GMWidgetResourceManager::onRenderRectResized()
{
	D(d);
	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
	d->backBufferWidth = windowStates.renderRect.width;
	d->backBufferHeight = windowStates.renderRect.height;
}

GMfloat GMWidget::s_timeRefresh = 0;
GMControl* GMWidget::s_controlFocus = nullptr;
GMControl* GMWidget::s_controlPressed = nullptr;

GMWidget::GMWidget(GMWidgetResourceManager* manager)
{
	D(d);
	d->manager = manager;
	d->nextWidget = d->prevWidget = this;
	initStyles();
}

GMWidget::~GMWidget()
{
	D(d);
	removeAllControls();
}

void GMWidget::addControl(GMControl* control)
{
	D(d);
	d->controls.push_back(control);
	bool b = initControl(control);
	GM_ASSERT(b);
}

const GMWidgetTextureArea& GMWidget::getArea(GMTextureArea::Area area)
{
	D(d);
	return d->areas[area];
}

GMStyle GMWidget::getTitleStyle()
{
	D(d);
	return d->titleStyle;
}

void GMWidget::setTitleStyle(const GMStyle& titleStyle)
{
	D(d);
	d->titleStyle = titleStyle;
}

void GMWidget::setTitleVisible(
	bool visible
)
{
	D(d);
	d->title = visible;
}

void GMWidget::setTitle(
	const GMString& text,
	const GMPoint& offset
)
{
	D(d);
	d->titleText = text;
	d->titleOffset = offset;
}

void GMWidget::addBorder(
	const GMRect& corner,
	const GMint32 marginLeft,
	const GMint32 marginTop
)
{
	D(d);
	d->borderMarginLeft = marginLeft;
	d->borderMarginTop = marginTop;

	addBorder(
		-marginLeft,
		-marginTop - getTitleHeight(),
		d->width + 2 * marginLeft,
		d->height + 2 * (getTitleHeight() + marginTop),
		corner,
		&d->borderControl
	);
}

void GMWidget::addBorder(
	GMint32 x,
	GMint32 y,
	GMint32 width,
	GMint32 height,
	const GMRect& cornerRect,
	OUT GMControlBorder** out
)
{
	D(d);
	GMControlBorder* borderControl = GMControlBorder::createControl(this, x, y, width, height, cornerRect);
	addControl(borderControl);
	if (out)
		*out = borderControl;
}

void GMWidget::drawText(
	const GMString& text,
	GMStyle& style,
	const GMRect& rc,
	bool shadow,
	bool center,
	bool newLine,
	GMint32 lineSpacing
)
{
	// 不需要绘制透明元素
	if (style.getFontColor().getCurrent().getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(targetRc);

	if (shadow)
	{
		const GMShadowStyle& shadowStyle = style.getShadowStyle();
		GMRect shadowRc = { rc.x + shadowStyle.offsetX, rc.y + shadowStyle.offsetY, rc.width, rc.height };
		drawText(
			text,
			d->shadowStyle,
			shadowRc,
			false,
			center,
			newLine,
			lineSpacing
		);
	}

	const GMVec4& fontColor = style.getFontColor().getCurrent();
	GMTextGameObject* textObject = d->manager->getTextObject();
	textObject->setDrawMode(GMTextDrawMode::Immediate);
	textObject->setColorType(GMTextColorType::Plain);
	textObject->setColor(fontColor);
	textObject->setText(text);
	textObject->setGeometry(targetRc);
	textObject->setCenter(center);
	textObject->setFont(style.getFont());
	textObject->setNewline(newLine);
	textObject->setLineSpacing(lineSpacing);
	textObject->draw();
}

void GMWidget::drawText(
	GMTypoTextBuffer* textBuffer,
	GMStyle& style,
	const GMRect& rc,
	bool shadow
)
{
	// 不需要绘制透明元素
	if (style.getFontColor().getCurrent().getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(targetRc);

	if (shadow)
	{
		const GMShadowStyle& shadowStyle = style.getShadowStyle();
		GMRect shadowRc = { rc.x + shadowStyle.offsetX, rc.y + shadowStyle.offsetY, rc.width, rc.height };
		drawText(
			textBuffer,
			d->shadowStyle,
			shadowRc,
			false
		);
	}

	const GMVec4& fontColor = style.getFontColor().getCurrent();
	GMTextGameObject* textObject = d->manager->getTextObject();
	textObject->setTextBuffer(textBuffer);
	textObject->setColorType(GMTextColorType::Plain);
	textObject->setColor(fontColor);
	textObject->setGeometry(targetRc);
	textObject->setFont(style.getFont());
	textObject->draw();
}

void GMWidget::drawSprite(
	GMStyle& style,
	const GMRect& rc,
	GMfloat depth
)
{
	// 不需要绘制透明元素
	if (style.getTextureColor().getCurrent().getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(targetRc);

	const GMRect& textureRc = style.getTextureRect();
	GMuint32 texId = style.getTexture();
	const GMCanvasTextureInfo& texInfo = d->manager->getTexture(style.getTexture());

	GMSprite2DGameObject* spriteObject = d->manager->getSpriteObject();
	spriteObject->setDepth(depth);
	spriteObject->setGeometry(targetRc);
	spriteObject->setTexture(texInfo.texture);
	spriteObject->setTextureRect(textureRc);
	spriteObject->setTextureSize(texInfo.width, texInfo.height);
	spriteObject->setColor(style.getTextureColor().getCurrent());
	spriteObject->draw();
}

void GMWidget::drawRect(
	const GMVec4& bkColor,
	const GMRect& rc,
	bool isOpaque,
	GMfloat depth
)
{
	// 不需要绘制透明元素
	if (bkColor.getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(targetRc);

	const GMRect& textureRc = d->whiteTextureStyle.getTextureRect();
	GMuint32 texId = d->whiteTextureStyle.getTexture();
	const GMCanvasTextureInfo& texInfo = d->manager->getTexture(d->whiteTextureStyle.getTexture());

	GMSprite2DGameObject* spriteObject = isOpaque ? d->manager->getOpaqueSpriteObject() : d->manager->getSpriteObject();
	spriteObject->setDepth(depth);
	spriteObject->setGeometry(targetRc);
	spriteObject->setTexture(texInfo.texture);
	spriteObject->setTextureRect(textureRc);
	spriteObject->setTextureSize(texInfo.width, texInfo.height);
	spriteObject->setColor(bkColor);
	spriteObject->draw();
}

void GMWidget::drawBorder(
	GMStyle& style,
	const GMRect& cornerRc,
	const GMRect& rc,
	GMfloat depth
)
{
	if (style.getTextureColor().getCurrent().getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(targetRc);

	const GMRect& textureRc = style.getTextureRect();
	GMuint32 texId = style.getTexture();
	const GMCanvasTextureInfo& texInfo = d->manager->getTexture(style.getTexture());

	GMBorder2DGameObject* borderObject = d->manager->getBorderObject();
	borderObject->setDepth(depth);
	borderObject->setGeometry(targetRc);
	borderObject->setTexture(texInfo.texture);
	borderObject->setTextureRect(textureRc);
	borderObject->setTextureSize(texInfo.width, texInfo.height);
	borderObject->setColor(style.getTextureColor().getCurrent());
	borderObject->setCornerRect(cornerRc);
	borderObject->draw();
}

void GMWidget::drawStencil(
	const GMRect& rc,
	GMfloat depth,
	bool drawRc,
	const GMVec4& color,
	bool clearCurrentStencil
)
{
	D(d);
	auto engine = d->parentWindow->getGraphicEngine();
	const auto& currentStencilOptions = engine->getStencilOptions();
	if (clearCurrentStencil)
		engine->getDefaultFramebuffers()->clear(GMFramebuffersClearType::Stencil);

	if (drawRc)
	{
		GMStencilOptions stencilOptions(GMStencilOptions::OxFF, GMStencilOptions::Always);
		engine->setStencilOptions(stencilOptions);
	}
	else
	{
		// 如果不用绘制矩形，模板测试一定要失败，并且要更新模板缓存
		GMStencilOptions stencilOptions(GMStencilOptions::OxFF, GMStencilOptions::Never, GMStencilOptions::Replace, GMStencilOptions::Keep, GMStencilOptions::Keep);
		engine->setStencilOptions(stencilOptions);
	}
	drawRect(color, rc, true, depth);
}

void GMWidget::useStencil(
	bool inside
)
{
	D(d);
	static GMStencilOptions s_inside(GMStencilOptions::OxFF, GMStencilOptions::Equal);
	static GMStencilOptions s_outside(GMStencilOptions::OxFF, GMStencilOptions::NotEqual);
	auto engine = d->parentWindow->getGraphicEngine();
	engine->setStencilOptions(inside ? s_inside : s_outside);
}

void GMWidget::endStencil()
{
	D(d);
	static GMStencilOptions s_stencilOptions(GMStencilOptions::Ox00, GMStencilOptions::Always);
	auto engine = d->parentWindow->getGraphicEngine();
	engine->getDefaultFramebuffers()->clear(GMFramebuffersClearType::Stencil);
	engine->setStencilOptions(s_stencilOptions);
}

void GMWidget::requestFocus(GMControl* control)
{
	if (s_controlFocus == control)
		return;

	if (!control->canHaveFocus())
		return;

	if (s_controlFocus)
		s_controlFocus->onFocusOut();

	control->onFocusIn();
	s_controlFocus = control;
}

void GMWidget::setSize(GMint32 width, GMint32 height)
{
	D(d);
	d->width = width;
	d->height = height;
	onUpdateSize();
}

bool GMWidget::verticalScroll(GMint32 offset)
{
	D(d);
	GMOverflowStyle overflow = getOverflow();
	if (overflow == GMOverflowStyle::Scroll || overflow == GMOverflowStyle::Auto)
	{
		GMint32 flag = CannotScroll;

		// 如果控件的包围盒高度超出内容区域，则滚动区域，设置一个scrollOffsetY
		// scrollOffsetY为负数表示向上滚动
		flag = getContentOverflowFlag();
		if ((offset < 0 && flag & CanScrollDown) || (offset > 0 && flag & CanScrollUp))
			d->scrollOffsetY += offset;

		GMRect contentRect = getContentRect();
		if (d->scrollOffsetY + d->controlBoundingBox.y + d->controlBoundingBox.height < contentRect.y + contentRect.height)
			d->scrollOffsetY = contentRect.y + contentRect.height - d->controlBoundingBox.y - d->controlBoundingBox.height;
		if (d->scrollOffsetY + d->controlBoundingBox.y > contentRect.y)
			d->scrollOffsetY = contentRect.y - d->controlBoundingBox.y;

		if (d->verticalScrollbar)
			d->verticalScrollbar->setValue(-d->scrollOffsetY);

		return true;
	}

	d->scrollOffsetY = 0;
	return false;
}

bool GMWidget::verticalScrollTo(GMint32 value)
{
	D(d);
	GMOverflowStyle overflow = getOverflow();
	if (overflow == GMOverflowStyle::Scroll || overflow == GMOverflowStyle::Auto)
	{
		d->scrollOffsetY = value;
		if (d->verticalScrollbar)
			d->verticalScrollbar->setValue(-d->scrollOffsetY);
		return true;
	}
	d->scrollOffsetY = 0;
	return false;
}

bool GMWidget::initControl(GMControl* control)
{
	D(d);
	GM_ASSERT(control);
	if (!control)
		return false;

	control->setIndex(gm_sizet_to_int(d->controls.size() - 1));
	return control->onInit();
}

void GMWidget::setPrevCanvas(GMWidget* prevWidget)
{
	D(d);
	d->prevWidget= prevWidget;
}

void GMWidget::addArea(GMTextureArea::Area area, GMlong textureId, const GMRect& rc)
{
	D(d);
	d->areas[area] = { textureId, rc };
}

bool GMWidget::msgProc(GMSystemEvent* event)
{
	D(d);
	if (!getVisible())
		return false;

	if (s_controlFocus &&
		s_controlFocus->getParent() == this &&
		s_controlFocus->getEnabled())
	{
		if (s_controlFocus->msgProc(event))
			return true;
	}

	GMSystemEventType type = event->getType();
	switch (type)
	{
	case GMSystemEventType::CaptureChanged:
	{
		if (s_controlFocus &&
			s_controlFocus->getParent() == this &&
			s_controlFocus->getEnabled())
		{
			if (s_controlFocus->onCaptureChanged(gm_cast<GMSystemCaptureChangedEvent*>(event)))
				return true;
		}
		break;
	}
	case GMSystemEventType::KeyDown:
	case GMSystemEventType::KeyUp:
	{
		GMSystemKeyEvent* keyEvent = gm_cast<GMSystemKeyEvent*>(event);
		if (s_controlFocus &&
			s_controlFocus->getParent() == this &&
			s_controlFocus->getEnabled())
		{
			if (s_controlFocus->handleKeyboard(keyEvent))
				return true;
		}

		if (type == GMSystemEventType::KeyDown)
		{
			if (!d->keyboardInput)
				return false;

			switch (keyEvent->getKey())
			{
			case GMKey_Right:
			case GMKey_Down:
			{
				if (s_controlFocus)
					return onCycleFocus(true);
				break;
			}
			case GMKey_Left:
			case GMKey_Up:
			{
				if (s_controlFocus)
					return onCycleFocus(false);
				break;
			}
			case GMKey_Tab:
				bool shiftDown = (keyEvent->getModifier() & GMModifier_Shift) != 0;
				return onCycleFocus(!shiftDown);
			}
		}
		break;
	}
	case GMSystemEventType::Char:
	{
		GMSystemCharEvent* keyEvent = gm_cast<GMSystemCharEvent*>(event);
		if (s_controlFocus &&
			s_controlFocus->getParent() == this &&
			s_controlFocus->getEnabled())
		{
			if (s_controlFocus->handleKeyboard(keyEvent))
				return true;
		}
		break;
	}
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDblClick:
	case GMSystemEventType::MouseWheel:
	{
		GMSystemMouseEvent* mouseEvent = gm_cast<GMSystemMouseEvent*>(event);
		GMPoint pt = mouseEvent->getPoint();
		pt.x -= d->x;
		pt.y -= d->y;

		GMSystemMouseWheelEvent cacheWheelEvent;
		GMSystemMouseEvent cacheEvent;

		GMSystemMouseEvent* pControlEvent = nullptr;
		if (type == GMSystemEventType::MouseWheel)
		{
			cacheWheelEvent = *(gm_cast<GMSystemMouseWheelEvent*>(mouseEvent));
			cacheWheelEvent.setPoint(pt);
			pControlEvent = &cacheWheelEvent;
		}
		else
		{
			cacheEvent = *mouseEvent;
			cacheEvent.setPoint(pt);
			pControlEvent = &cacheEvent;
		}

		// 判断是否拖拽Widget
		if (type == GMSystemEventType::MouseDown)
		{
			if (d->title)
			{
				const GMPoint& pt = cacheEvent.getPoint();
				GMRect rcBound = { 0, -d->titleHeight, d->width, d->titleHeight };
				if (GM_inRect(rcBound, pt) && onTitleMouseDown(&cacheEvent))
					return true;
			}
		}
		else if (type == GMSystemEventType::MouseMove)
		{
			if (d->title && onTitleMouseMove(&cacheEvent))
				return true;
		}
		else if (type == GMSystemEventType::MouseUp)
		{
			if (d->title && onTitleMouseUp(&cacheEvent))
				return true;
		}
		else if (type == GMSystemEventType::MouseWheel)
		{
			if (onMouseWheel(&cacheWheelEvent))
				return true;
		}

		if (s_controlFocus &&
			s_controlFocus->getParent() == this &&
			s_controlFocus->getEnabled())
		{
			if (s_controlFocus->handleMouse(pControlEvent))
				return true;
		}

		// 点击测试，找到鼠标所在位置的控件
		GMControl* control = getControlAtPoint(pt);
		if (control && control->getEnabled())
		{
			if (control->handleMouse(pControlEvent))
				return true;
		}
		else
		{
			// 如果没有找到任何控件，那么当前的焦点控件失去焦点
			if (type == GMSystemEventType::MouseDown && 
				s_controlFocus &&
				s_controlFocus->getParent() == this)
			{
				s_controlFocus->onFocusOut();
				s_controlFocus = nullptr;
			}
		}

		// 仍然没有处理
		if (type == GMSystemEventType::MouseMove)
		{
			onMouseMove(pt);
			return false;
		}
		break;
	}
	}
	return false;
}

bool GMWidget::onTitleMouseDown(const GMSystemMouseEvent* event)
{
	D(d);
	if (!d->movingWidget)
	{
		d->movingWidget = true;
		d->movingStartPt = event->getPoint();
		return true;
	}
	return false;
}

bool GMWidget::onTitleMouseMove(const GMSystemMouseEvent* event)
{
	D(d);
	if (d->movingWidget)
	{
		const GMPoint& pt = event->getPoint();
		GMPoint deltaDistance = { pt.x - d->movingStartPt.x, pt.y - d->movingStartPt.y };
		d->x += deltaDistance.x;
		d->y += deltaDistance.y;
		return true;
	}
	return false;
}

bool GMWidget::onTitleMouseUp(const GMSystemMouseEvent* event)
{
	D(d);
	if (d->movingWidget)
		d->movingWidget = false;
	return false;
}

bool GMWidget::onMouseWheel(const GMSystemMouseEvent* event)
{
	D(d);
	const GMSystemMouseWheelEvent* wheelEvent = gm_cast<const GMSystemMouseWheelEvent*>(event);
	// wheelStep为负数，表示滚轮向下滚动，为正数表示向上
	GMint32 wheelStep = wheelEvent->getDelta() / GM_WHEEL_DELTA * getScrollStep();
	return verticalScroll(wheelStep);
}

void GMWidget::onRenderTitle()
{
	D(d);
	GMRect rc = { 0, -d->titleHeight, d->width, d->titleHeight };
	drawSprite(d->titleStyle, rc, .99f);
	rc.x += d->titleOffset.x;
	rc.y += d->titleOffset.y;
	drawText(d->titleText, d->titleStyle, rc);
}

void GMWidget::onUpdateSize()
{
	D(d);
	if (d->borderControl)
	{
		d->borderControl->setPosition(-d->borderMarginLeft, -d->borderMarginTop - getTitleHeight());
		d->borderControl->setSize(d->width + 2 * d->borderMarginLeft, d->height + 2 * (getTitleHeight() + d->borderMarginTop));
	}
}

void GMWidget::onControlRectChanged(GMControl* control)
{
	D(d);
	// 重新计算所有控件的并集
	GMRect b = { 0 };
	if (control != d->borderControl)
	{
		for (auto c : d->controls)
		{
			if (c != d->borderControl)
				b = GM_unionRect(c->getBoundingRect(), b);
		}
	}
	d->controlBoundingBox = b;
	updateVerticalScrollbar();
}

void GMWidget::render(GMfloat elpasedTime)
{
	D(d);
	struct DataSpin
	{
		DataSpin(GMint32& ref, GMfloat v) : data(ref), cache(ref) { data = v; }
		~DataSpin() { data = cache; }
		GMint32& data;
		GMfloat cache;
	};

	if (d->timeLastRefresh < s_timeRefresh)
	{
		d->timeLastRefresh = GM.getRunningStates().elapsedTime;
		refresh();
	}

	if (!d->visible ||
		(d->minimized && !d->title))
		return;

	// 更新滚动条状态
	if (needShowVerticalScrollbar())
		createVerticalScrollbar();
	else
		disableVerticalScrollbar();

	// 如果overflow样式为hidden，那么我们要在Widget的标题栏下方绘制一块模板
	// 这样，超出的部分将不会被绘制出来
	// 如果overflow样式为auto，那么我们不仅要在Widget的标题栏下方绘制一块模板，还需要多绘制一个滚动条
	// 如果overflow样式为visible，允许内容溢出边框
	if (getOverflow() != GMOverflowStyle::Visible)
	{
		// 计算显示内容的矩形
		DataSpin ds(d->scrollOffsetY, 0);
		GMRect rc = getContentRect();
		drawStencil(rc, .99f, false);
		useStencil(true);
	}

	if (!d->minimized)
	{
		for (auto control : d->controls)
		{
			// 我们先不绘制边框、滚动条，最后再绘制
			if (d->borderControl == control || 
				d->verticalScrollbar.get() == control)
				continue;

			// 最后渲染焦点控件
			if (control == d->focusControl)
				continue;

			control->render(elpasedTime);
		}

		if (d->focusControl && d->focusControl->getParent() == this)
			d->focusControl->render(elpasedTime);
	}

	if (getOverflow() == GMOverflowStyle::Auto || getOverflow() == GMOverflowStyle::Hidden)
	{
		endStencil();
	}

	// 最后绘制不随滚动状态而变化的部分，如边框，标题栏
	{
		DataSpin ds(d->scrollOffsetY, 0);
		if (d->title)
			onRenderTitle();

		if (!d->minimized)
		{
			d->borderControl->render(elpasedTime);
			if (d->verticalScrollbar)
				d->verticalScrollbar->render(elpasedTime);
		}
	}
}

void GMWidget::setNextWidget(GMWidget* nextWidget)
{
	D(d);
	if (!nextWidget)
		nextWidget = this;
	d->nextWidget = nextWidget;
	if (nextWidget)
		nextWidget->setPrevCanvas(this);
}

void GMWidget::refresh()
{
	D(d);
	if (s_controlFocus)
		s_controlFocus->onFocusOut();

	if (d->controlMouseOver)
		d->controlMouseOver->onMouseLeave();

	s_controlFocus = nullptr;
	s_controlPressed = nullptr;
	resetControlMouseOver();

	for (auto control : d->controls)
	{
		control->refresh();
	}

	if (d->keyboardInput)
		focusDefaultControl();
}

void GMWidget::focusDefaultControl()
{
	D(d);
	for (auto& control : d->controls)
	{
		if (control->isDefault())
		{
			clearFocus(this);

			s_controlFocus = control;
			s_controlFocus->onFocusIn();
			return;
		}
	}
}

void GMWidget::removeAllControls()
{
	D(d);
	if (s_controlFocus && s_controlFocus->getParent() == this)
		s_controlFocus = nullptr;
	if (s_controlPressed && s_controlPressed->getParent() == this)
		s_controlPressed = nullptr;
	resetControlMouseOver();

	for (auto control : d->controls)
	{
		GM_delete(control);
	}
	GMClearSTLContainer(d->controls);
}

GMControl* GMWidget::getControlAtPoint(GMPoint pt)
{
	D(d);
	// 响应scrollbars
	if (d->verticalScrollbar)
	{
		if (d->verticalScrollbar->getEnabled() && d->verticalScrollbar->getVisible() && d->verticalScrollbar->containsPoint(pt))
			return d->verticalScrollbar.get();
	}

	// 在overflow样式为非visible时，不处理内容区域之外的流控件(PositonFlag == Auto)，因为它们已经被遮挡
	if (getOverflow() != GMOverflowStyle::Visible && !GM_inRect(getContentRect(), pt))
		return nullptr;

	for (auto control : d->controls)
	{
		if (!control)
			continue;

		if (control->getEnabled() && control->getVisible() && control->containsPoint(pt))
			return control;
	}

	return nullptr;
}

bool GMWidget::onCycleFocus(bool goForward)
{
	D(d);
	GMControl* control = nullptr;
	GMWidget* widget = nullptr;
	GMWidget* lastCanvas = nullptr;
	const Vector<GMWidget*>& widgets = d->manager->getCanvases();
	GMint32 sz = (GMint32)widgets.size();

	if (!s_controlFocus)
	{
		if (goForward)
		{
			for (GMint32 i = 0; i < sz; ++i)
			{
				widget = lastCanvas = widgets[i];
				const Vector<GMControl*> controls = widget->getControls();
				if (widget && controls.size() > 0)
				{
					control = controls[0];
					break;
				}
			}
		}
		else
		{
			for (GMint32 i = sz - 1; i >= 0; --i)
			{
				widget = lastCanvas = widgets[i];
				const Vector<GMControl*> controls = widget->getControls();
				if (widget && controls.size() > 0)
				{
					control = controls[controls.size() - 1];
					break;
				}
			}
		}

		if (!widget || !control)
			return true;
	}
	else if (s_controlFocus->getParent() != this)
	{
		// 当前获得焦点的控件属于另外一个widget，所以要交给它来处理
		return false;
	}
	else
	{
		lastCanvas = s_controlFocus->getParent();
		control = (goForward) ? getNextControl(s_controlFocus) : getPrevControl(s_controlFocus);
		widget = control->getParent();
	}

	while (true)
	{
		// 如果我们转了一圈回来，那么我们不会设置任何焦点了。
		const Vector<GMWidget*> widgets = d->manager->getCanvases();
		if (widgets.empty())
			return false;

		GMsize_t lastCanvasIndex = indexOf(widgets, lastCanvas);
		GMsize_t canvasIndex = indexOf(widgets, widget);
		if ((!goForward && lastCanvasIndex < canvasIndex) ||
			(goForward && canvasIndex < lastCanvasIndex))
		{
			if (s_controlFocus)
				s_controlFocus->onFocusOut();
			s_controlFocus = nullptr;
			return true;
		}

		if (control == s_controlFocus)
			return true;

		if (control->getParent()->canKeyboardInput() && control->canHaveFocus())
		{
			if (s_controlFocus)
				s_controlFocus->onFocusOut();
			s_controlFocus = control;
			s_controlFocus->onFocusIn();
			return true;
		}

		lastCanvas = widget;
		control = (goForward) ? getNextControl(control) : getPrevControl(control);
		widget = control->getParent();
	}

	// 永远都不会到这里来，因为widget是个环，只会在上面return
	GM_ASSERT(false);
	return false;
}

void GMWidget::onMouseMove(const GMPoint& pt)
{
	D(d);
	GMControl* control = getControlAtPoint(pt);
	// 停留在相同控件上，不需要触发什么事件
	if (control == d->controlMouseOver)
		return;

	if (d->controlMouseOver)
		d->controlMouseOver->onMouseLeave();

	d->controlMouseOver = control;
	if (control)
		control->onMouseEnter();
}

void GMWidget::mapRect(GMRect& rc)
{
	D(d);
	rc.x += d->x;
	rc.y += d->y;
	rc.y += d->scrollOffsetY; // 可能存在滚动
}

void GMWidget::initStyles()
{
	D(d);
	GMStyle titleStyle;
	titleStyle.setFont(0);
	titleStyle.setTexture(getArea(GMTextureArea::CaptionArea));
	titleStyle.setTextureColor(GMControlState::Normal, GMVec4(1, 1, 1, 1));
	titleStyle.setFontColor(GMControlState::Normal, GMVec4(1, 1, 1, 1));
	titleStyle.getTextureColor().blend(GMControlState::Normal, .5f);
	titleStyle.getFontColor().blend(GMControlState::Normal, 1.f);
	d->titleStyle = std::move(titleStyle);

	GMStyle shadowStyle;
	shadowStyle.getFontColor().setCurrent(GMVec4(0, 0, 0, 1));
	d->shadowStyle = shadowStyle;

	GMStyle whiteTextureStyle;
	GMRect rc = { 0, 0, 1, 1 };
	whiteTextureStyle.setTexture({ d->manager->getWhiteTextureId(), rc });
	d->whiteTextureStyle = whiteTextureStyle;
}

GMRect GMWidget::getContentRect()
{
	D(d);
	GMRect rc = { d->contentPaddingLeft, d->contentPaddingTop, d->width - d->contentPaddingRight, d->height - d->contentPaddingBottom };
	if (d->verticalScrollbar && d->verticalScrollbar->getVisible())
		rc.width -= d->verticalScrollbarWidth;

	return rc;
}

GMint32 GMWidget::getContentOverflowFlag()
{
	D(d);
	GMint32 flag = CannotScroll;
	GMRect contentRect = getContentRect();
	if (d->scrollOffsetY + d->controlBoundingBox.y + d->controlBoundingBox.height > contentRect.y + contentRect.height)
		flag |= CanScrollDown;
	if (d->scrollOffsetY + d->controlBoundingBox.y < contentRect.y)
		flag |= CanScrollUp;
	return flag;
}

void GMWidget::createVerticalScrollbar()
{
	D(d);
	if (!d->verticalScrollbar)
	{
		GMRect contentRect = getContentRect();
		d->verticalScrollbar.reset(GMControlScrollBar::createControl(
			this,
			d->width - d->borderMarginLeft - d->verticalScrollbarWidth,
			contentRect.y,
			d->verticalScrollbarWidth,
			contentRect.height,
			false,
			d->scrollbarThumbCorner
		));

		d->verticalScrollbar->setPositionFlag(GMControlPositionFlag::Fixed); // 不随Widget滚动条而移动
		connect(*d->verticalScrollbar, GMControlScrollBar::valueChanged, [=](auto sender, auto receiver) {
			this->verticalScrollTo(-gm_cast<GMControlScrollBar*>(sender)->getValue());
		});
		updateVerticalScrollbar();
	}
}

void GMWidget::updateVerticalScrollbar()
{
	D(d);
	if (d->verticalScrollbar)
	{
		d->verticalScrollbar->setMinimum(0);
		d->verticalScrollbar->setPageStep(getContentRect().height);
		d->verticalScrollbar->setMaximum(d->controlBoundingBox.height - d->verticalScrollbar->getPageStep());
		d->verticalScrollbar->setSingleStep(getScrollStep());
		d->verticalScrollbar->setVisible(true);
	}
}

void GMWidget::disableVerticalScrollbar()
{
	D(d);
	if (d->verticalScrollbar)
	{
		d->verticalScrollbar->setVisible(false);
		updateVerticalScrollbar();
	}
}

bool GMWidget::needShowVerticalScrollbar()
{
	GMint32 overflowFlag = getContentOverflowFlag();
	return (overflowFlag & CanScrollUp || overflowFlag & CanScrollDown && getOverflow() != GMOverflowStyle::Visible);
}

void GMWidget::clearFocus(GMWidget* sender)
{
	if (s_controlFocus)
	{
		s_controlFocus->onFocusOut();
		s_controlFocus = nullptr;
	}

	IWindow* window = sender->getParentWindow();
	if (window)
		window->setWindowCapture(false);
}