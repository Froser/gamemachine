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
		GMuint index = control->getIndex() + 1;

		// 如果下一个控件不在此画布内，则跳到下一个画布进行查找
		while (index >= (GMuint)parentWidget->getControls().size())
		{
			parentWidget = parentWidget->getNextCanvas();
			index = 0;
		}

		return parentWidget->getControls()[index];
	}

	GMControl* getPrevControl(GMControl* control)
	{
		GMWidget* parentCanvas = control->getParent();
		GMsize_t index = (GMint) control->getIndex() - 1;
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

GMlong GMWidgetResourceManager::addTexture(GMAsset texture, GMint width, GMint height)
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

void GMWidget::addLabel(
	const GMString& text,
	const GMVec4& fontColor,
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	bool isDefault,
	OUT GMControlLabel** out
)
{
	GMControlLabel* labelControl = new GMControlLabel(this);
	if (out)
		*out = labelControl;

	addControl(labelControl);
	labelControl->setText(text);
	labelControl->setPosition(x, y);
	labelControl->setSize(width, height);
	labelControl->setFontColor(fontColor);
	labelControl->setIsDefault(isDefault);
}

void GMWidget::addButton(
	const GMString& text,
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	bool isDefault,
	OUT GMControlButton** out
)
{
	GMControlButton* buttonControl = new GMControlButton(this);
	if (out)
		*out = buttonControl;

	addControl(buttonControl);
	buttonControl->setText(text);
	buttonControl->setPosition(x, y);
	buttonControl->setSize(width, height);
	buttonControl->setIsDefault(isDefault);
}

void GMWidget::addBorder(
	const GMRect& corner,
	const GMint marginLeft,
	const GMint marginTop
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
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	const GMRect& cornerRect,
	OUT GMControlBorder** out
)
{
	D(d);
	GMControlBorder* borderControl = new GMControlBorder(this);
	if (out)
		*out = borderControl;

	addControl(borderControl);
	borderControl->setPosition(x, y);
	borderControl->setSize(width, height);
	borderControl->setCorner(cornerRect);
}

void GMWidget::addTextEdit(
	const GMString& text,
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	bool isDefault,
	const GMRect& cornerRect,
	OUT GMControlTextEdit** out
)
{
	GMControlTextEdit* textEdit= new GMControlTextEdit(this);
	if (out)
		*out = textEdit;

	addControl(textEdit);
	textEdit->setText(text);
	textEdit->setPosition(x, y);
	textEdit->setSize(width, height);
	textEdit->setIsDefault(isDefault);

	GMControlBorder* border = textEdit->getBorder();
	border->setCorner(cornerRect);
}

void GMWidget::addTextArea(
	const GMString& text,
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	bool isDefault,
	bool hasScrollBar,
	const GMRect& textAreaCornerRect,
	const GMRect& scrollBarThumbCornerRect,
	OUT GMControlTextArea** out
)
{
	GMControlTextArea* textArea = new GMControlTextArea(this);
	if (out)
		*out = textArea;

	addControl(textArea);
	textArea->setText(text);
	textArea->setPosition(x, y);
	textArea->setSize(width, height);
	textArea->setIsDefault(isDefault);
	textArea->setScrollBar(hasScrollBar);
	if (hasScrollBar)
		textArea->getScrollBar()->setThumbCorner(scrollBarThumbCornerRect);

	GMControlBorder* border = textArea->getBorder();
	border->setCorner(textAreaCornerRect);

	if (out)
		*out = textArea;
}

void GMWidget::addScrollBar(
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	bool isDefault,
	const GMRect& scrollBarThumbCornerRect,
	OUT GMControlScrollBar** out
)
{
	GMControlScrollBar* scrollBar = new GMControlScrollBar(this);
	if (out)
		*out = scrollBar;

	addControl(scrollBar);
	scrollBar->setPosition(x, y);
	scrollBar->setSize(width, height);
	scrollBar->setIsDefault(isDefault);
	scrollBar->setThumbCorner(scrollBarThumbCornerRect);

	if (out)
		*out = scrollBar;
}

void GMWidget::drawText(
	const GMString& text,
	GMStyle& style,
	const GMRect& rc,
	bool shadow,
	bool center,
	bool newLine,
	GMint lineSpacing
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
	GMuint texId = style.getTexture();
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
	GMuint texId = d->whiteTextureStyle.getTexture();
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
	GMuint texId = style.getTexture();
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
	bool clear
)
{
	D(d);
	static GMVec4 one = GMVec4(1, 1, 1, 1);
	static GMStencilOptions s_stencilOptions(GMStencilOptions::OxFF, GMStencilOptions::Always);
	auto engine = d->parentWindow->getGraphicEngine();
	if (clear)
		engine->getDefaultFramebuffers()->clear(GMFramebuffersClearType::Stencil);
	engine->setStencilOptions(s_stencilOptions);
	drawRect(one, rc, true, depth);
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

void GMWidget::setSize(GMint width, GMint height)
{
	D(d);
	d->width = width;
	d->height = height;
	onUpdateSize();
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

void GMWidget::render(GMfloat elpasedTime)
{
	D(d);
	if (d->timeLastRefresh < s_timeRefresh)
	{
		d->timeLastRefresh = GM.getRunningStates().elapsedTime;
		refresh();
	}

	if (!d->visible ||
		(d->minimized && !d->title))
		return;

	if (d->title)
		onRenderTitle();

	if (!d->minimized)
	{
		for (auto control : d->controls)
		{
			// 最后渲染焦点控件
			if (control == d->focusControl)
				continue;

			control->render(elpasedTime);
		}

		if (d->focusControl && d->focusControl->getParent() == this)
			d->focusControl->render(elpasedTime);
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
	d->controlMouseOver = nullptr;

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
	d->controlMouseOver = nullptr;

	for (auto control : d->controls)
	{
		GM_delete(control);
	}
	GMClearSTLContainer(d->controls);
}

GMControl* GMWidget::getControlAtPoint(const GMPoint& pt)
{
	D(d);
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
	GMint sz = (GMint)widgets.size();

	if (!s_controlFocus)
	{
		if (goForward)
		{
			for (GMint i = 0; i < sz; ++i)
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
			for (GMint i = sz - 1; i >= 0; --i)
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