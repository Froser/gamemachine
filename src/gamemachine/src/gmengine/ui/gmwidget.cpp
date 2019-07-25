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
#include "gmuiconfiguration.h"
#include "gmwidget_p.h"

BEGIN_NS

namespace
{
	// 维护一个栈，用来弹出模板缓存
	struct StencilOptionsPair
	{
		GMStencilOptions begin;
		GMStencilOptions use;
	};

	struct StencilCache
	{
		GMRect rc;
		StencilOptionsPair stencilOptions;
		GMVec4 color;
		GMfloat depth;
	};

	struct StencilOptionsPool
	{
		Stack<StencilCache, Deque<StencilCache, AlignedAllocator<StencilCache>>> stencilCaches;
		static GMbyte nestLevel;
	};
	GMbyte StencilOptionsPool::nestLevel = 0;
	static StencilOptionsPool s_stencilPool;

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
			index = gm_sizet_to_int(parentCanvas->getControls().size() - 1);
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

GM_PRIVATE_OBJECT_ALIGNED(GMElementBlendColor)
{
	GMVec4 states[GMControlState::EndOfControlState];
	GMVec4 current;
};

GMElementBlendColor::GMElementBlendColor()
{
	GM_CREATE_DATA();
}

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

GMVec4* GMElementBlendColor::getStates()
{
	D(d);
	return d->states;
}

void GMElementBlendColor::setCurrent(const GMVec4& current)
{
	D(d);
	d->current = current;
}

const GMVec4& GMElementBlendColor::getCurrent()
{
	D(d);
	return d->current;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMStyle)
{
	GMlong texture;
	GMFontHandle font = 0;
	GMRect rc;
	GMRect cornerRc;
	GMElementBlendColor textureColor;
	GMElementBlendColor fontColor;
	GMShadowStyle shadowStyle;
};

GM_DEFINE_PROPERTY(GMStyle, GMRect, CornerRc, cornerRc)
GMStyle::GMStyle(
	const GMVec4& defaultTextureColor,
	const GMVec4& disabledTextureColor,
	const GMVec4& hiddenTextureColor
)
{
	GM_CREATE_DATA();

	D(d);
	resetTextureColor(defaultTextureColor, disabledTextureColor, hiddenTextureColor);
}

GMStyle::~GMStyle()
{

}


GMStyle::GMStyle(const GMStyle& rhs)
{
	*this = rhs;
}

GMStyle::GMStyle(GMStyle&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMStyle& GMStyle::operator=(const GMStyle& rhs)
{
	GM_COPY(rhs);
	return *this;
}

GMStyle& GMStyle::operator=(GMStyle&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}


void GMStyle::setTexture(const GMWidgetTextureArea& idRc)
{
	D(d);
	d->texture = idRc.textureId;
	d->rc = idRc.rc;
	d->cornerRc = idRc.cornerRc;
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

const GMShadowStyle& GMStyle::getShadowStyle()
{
	D(d);
	return d->shadowStyle;
}

void GMStyle::setShadowStyle(const GMShadowStyle& shadowStyle)
{
	D(d);
	d->shadowStyle = shadowStyle;
}

GMlong GMStyle::getTexture()
{
	D(d);
	return d->texture;
}

const GMRect& GMStyle::getTextureRect()
{
	D(d);
	return d->rc;
}

GMFontHandle GMStyle::getFont()
{
	D(d);
	return d->font;
}

GMElementBlendColor& GMStyle::getFontColor()
{
	D(d);
	return d->fontColor;
}

GMElementBlendColor& GMStyle::getTextureColor()
{
	D(d);
	return d->textureColor;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMWidgetResourceManager)
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
	GMUIConfiguration* configuration = nullptr;
};

GMWidgetResourceManager::GMWidgetResourceManager(const IRenderContext* context)
{
	GM_CREATE_DATA();

	D(d);
	d->configuration = new GMUIConfiguration(context);
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

	GMAsset emptyTexture;
	GM.getFactory()->createEmptyTexture(context, emptyTexture);
	addTexture(emptyTexture, 1, 1); //放入一个空texture，占用textureId=0的情况

	GM.getFactory()->createWhiteTexture(context, d->whiteTexture);
	d->whiteTextureId = addTexture(d->whiteTexture, 1, 1);
}

GMWidgetResourceManager::~GMWidgetResourceManager()
{
	D(d);
	GM_delete(d->configuration);
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

const IRenderContext* GMWidgetResourceManager::getContext()
{
	D(d);
	return d->context;
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

GMWidget* GMWidgetResourceManager::createWidget()
{
	D(d);
	GMWidget* w = new GMWidget(this);
	d->configuration->initWidget(w);
	return w;
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

void GMWidgetResourceManager::setUIConfiguration(const GMUIConfiguration& config)
{
	D(d);
	*(d->configuration) = config;
}

GMlong GMWidgetResourceManager::getWhiteTextureId() GM_NOEXCEPT
{
	D(d);
	return d->whiteTextureId;
}

GMBorder2DGameObject* GMWidgetResourceManager::getBorderObject()
{
	D(d);
	return d->borderObject.get();
}

GMSprite2DGameObject* GMWidgetResourceManager::getOpaqueSpriteObject()
{
	D(d);
	return d->opaqueSpriteObject.get();
}

GMSprite2DGameObject* GMWidgetResourceManager::getSpriteObject()
{
	D(d);
	return d->spriteObject.get();
}

GMTextGameObject* GMWidgetResourceManager::getTextObject()
{
	D(d);
	return d->textObject.get();
}

GMint32 GMWidgetResourceManager::getBackBufferHeight()
{
	D(d);
	return d->backBufferHeight;
}

GMint32 GMWidgetResourceManager::getBackBufferWidth()
{
	D(d);
	return d->backBufferWidth;
}

const Vector<GMWidget*>& GMWidgetResourceManager::getCanvases()
{
	D(d);
	return d->widgets;
}

GMfloat GMWidget::s_timeRefresh = 0;
GMControl* GMWidget::s_controlFocus = nullptr;
GMControl* GMWidget::s_controlPressed = nullptr;
GM_DEFINE_PROPERTY(GMWidget, bool, Minimum, minimized)
GM_DEFINE_PROPERTY(GMWidget, bool, Visible, visible)
GM_DEFINE_PROPERTY(GMWidget, GMint32, ScrollStep, scrollStep)
GM_DEFINE_PROPERTY(GMWidget, GMint32, ScrollOffsetY, scrollOffsetY)
GM_DEFINE_PROPERTY(GMWidget, GMint32, VerticalScrollbarWidth, verticalScrollbarWidth)
GMWidget::GMWidget(GMWidgetResourceManager* manager)
{
	D(d);
	d->manager = manager;
	d->nextWidget = d->prevWidget = this;
	initStyles();
	updateScaling();
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
	if (!text.isEmpty())
		setTitleVisible(true);
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
	borderControl->setPositionFlag(GMControlPositionFlag::Fixed);
	addControl(borderControl);
	if (out)
		*out = borderControl;
}

void GMWidget::drawText(
	GMControlPositionFlag positionFlag,
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
	mapRect(positionFlag, targetRc);

	if (shadow)
	{
		const GMShadowStyle& shadowStyle = style.getShadowStyle();
		GMRect shadowRc = { rc.x + shadowStyle.offsetX, rc.y + shadowStyle.offsetY, rc.width, rc.height };
		drawText(
			positionFlag,
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
	GMControlPositionFlag positionFlag,
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
	mapRect(positionFlag, targetRc);

	if (shadow)
	{
		const GMShadowStyle& shadowStyle = style.getShadowStyle();
		GMRect shadowRc = { rc.x + shadowStyle.offsetX, rc.y + shadowStyle.offsetY, rc.width, rc.height };
		drawText(
			positionFlag,
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
	GMControlPositionFlag positionFlag,
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
	mapRect(positionFlag, targetRc);

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
	GMControlPositionFlag positionFlag,
	const GMVec4& bkColor,
	const GMRect& rc,
	bool isOpaque,
	GMfloat depth
)
{
	// 不需要绘制透明元素
	if (bkColor.getW() == 0)
		return;

	if (rc.width <= 0 || rc.height <= 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(positionFlag, targetRc);

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
	GMControlPositionFlag positionFlag,
	GMStyle& style,
	const GMRect& rc,
	GMfloat depth
)
{
	if (style.getTextureColor().getCurrent().getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(positionFlag, targetRc);

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
	borderObject->setCornerRect(style.getCornerRc());
	borderObject->draw();
}

void GMWidget::drawStencil(
	GMControlPositionFlag positionFlag,
	GMRect rc,
	GMfloat depth,
	bool drawRc,
	const GMVec4& color
)
{
	D(d);
	auto engine = d->parentWindow->getGraphicEngine();
	engine->getDefaultFramebuffers()->clear(GMFramebuffersClearType::Stencil);
	if (s_stencilPool.nestLevel > 0)
	{
		// 如果是嵌套模板，与之前的模板求交集
		// 需要将父容器的滚动条因素纳入考虑
		if (positionFlag == GMControlPositionFlag::Auto)
			rc.y += d->scrollOffsetY;
		GM_ASSERT(!s_stencilPool.stencilCaches.empty());
		rc = GM_intersectRect(rc, s_stencilPool.stencilCaches.top().rc);
	}

	// 记录下这次的绘制行为，在下一层endStencil时，还原
	GMStencilOptions stencilOptions = drawRc ?
		GMStencilOptions(
			GMStencilOptions::OxFF,
			GMStencilOptions::Always
		) :
		GMStencilOptions(
			GMStencilOptions::OxFF,
			GMStencilOptions::Never,
			GMStencilOptions::Replace,
			GMStencilOptions::Keep,
			GMStencilOptions::Keep
		);

	engine->setStencilOptions(stencilOptions);
	drawRect(GMControlPositionFlag::Fixed, color, rc, true, depth);

	StencilCache cache = { rc, { stencilOptions }, color, depth };
	s_stencilPool.stencilCaches.push(cache);
	++s_stencilPool.nestLevel;
}

void GMWidget::useStencil(
	bool inside
)
{
	D(d);
	auto engine = d->parentWindow->getGraphicEngine();
	GMStencilOptions stencilOptions = inside ? GMStencilOptions(GMStencilOptions::OxFF, GMStencilOptions::Equal) :
		GMStencilOptions(GMStencilOptions::OxFF, GMStencilOptions::NotEqual);
	engine->setStencilOptions(stencilOptions);
	// 缓存这次StencilOptions
	auto& cache = s_stencilPool.stencilCaches.top();
	cache.stencilOptions.use = stencilOptions;
}

void GMWidget::endStencil()
{
	D(d);
	--s_stencilPool.nestLevel;

	// 弹出当前的stencil，回到上一层
	auto engine = d->parentWindow->getGraphicEngine();
	engine->getDefaultFramebuffers()->clear(GMFramebuffersClearType::Stencil);

	// 丢弃本次drawStencil时的缓存
	s_stencilPool.stencilCaches.pop();
	if (s_stencilPool.stencilCaches.empty())
	{
		static GMStencilOptions s_stencilOptions(GMStencilOptions::Ox00, GMStencilOptions::Always);
		GM_ASSERT(s_stencilPool.nestLevel == 0);
		engine->setStencilOptions(s_stencilOptions);
	}
	else
	{
		// 重新绘制出之前保存的矩形区域，还原上一次的模板
		auto engine = d->parentWindow->getGraphicEngine();
		const auto& cache = s_stencilPool.stencilCaches.top();
		engine->setStencilOptions(cache.stencilOptions.begin);
		drawRect(GMControlPositionFlag::Fixed, cache.color, cache.rc, true, cache.depth);

		// 还原上一次使用中的模板选项
		engine->setStencilOptions(s_stencilPool.stencilCaches.top().stencilOptions.use);
	}
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
	GMint32 overflowFlag = getContentOverflowFlag();
	if ((overflowFlag & CanScrollUp) || (overflowFlag & CanScrollDown) )
	{
		GMOverflowStyle overflow = getOverflow();
		if (overflow == GMOverflowStyle::Scroll || overflow == GMOverflowStyle::Auto)
		{
			d->scrollOffsetY = value;
			if (d->verticalScrollbar)
				d->verticalScrollbar->setValue(-d->scrollOffsetY);
			return true;
		}
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

void GMWidget::addArea(GMTextureArea::Area area, GMlong textureId, const GMRect& rc, const GMRect& cornerRc)
{
	D(d);
	d->areas[area] = { textureId, rc, cornerRc };
}

bool GMWidget::handleSystemEvent(GMSystemEvent* event)
{
	D(d);
	if (!getVisible())
		return false;

	if (s_controlFocus &&
		s_controlFocus->getParent() == this &&
		s_controlFocus->getEnabled())
	{
		if (s_controlFocus->handleSystemEvent(event))
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
			if (s_controlFocus->onCaptureChanged(static_cast<GMSystemCaptureChangedEvent*>(event)))
				return true;
		}
		break;
	}
	case GMSystemEventType::KeyDown:
	case GMSystemEventType::KeyUp:
	{
		GMSystemKeyEvent* keyEvent = static_cast<GMSystemKeyEvent*>(event);
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
		GMSystemCharEvent* keyEvent = static_cast<GMSystemCharEvent*>(event);
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
		GM_ASSERT(!FuzzyCompare(d->scaling[0], 0) && !FuzzyCompare(d->scaling[1], 0));
		GMSystemMouseEvent* mouseEvent = static_cast<GMSystemMouseEvent*>(event);
		GMPoint pt = mouseEvent->getPoint();
		pt.x -= d->x * d->scaling[0];
		pt.y -= d->y * d->scaling[1];
		pt.x /= d->scaling[0];
		pt.y /= d->scaling[1];

		GMSystemMouseWheelEvent cacheWheelEvent;
		GMSystemMouseEvent cacheEvent;

		GMSystemMouseEvent* pControlEvent = nullptr;
		if (type == GMSystemEventType::MouseWheel)
		{
			cacheWheelEvent = *(static_cast<GMSystemMouseWheelEvent*>(mouseEvent));
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
			GMPoint pt = pControlEvent->getPoint();
			if (s_controlFocus->handleMouse(adjustMouseEvent(pControlEvent, s_controlFocus)))
				return true;
			pControlEvent->setPoint(pt);
		}

		// 点击测试，找到鼠标所在位置的控件
		GMControl* control = getControlAtPoint(pt);
		if (control && control->getEnabled())
		{
			GMPoint pt = pControlEvent->getPoint();
			if (control->handleMouse(adjustMouseEvent(pControlEvent, control)))
				return true;
			pControlEvent->setPoint(pt);
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
		else if (type == GMSystemEventType::MouseWheel)
		{
			if (onMouseWheel(pControlEvent))
				return true;
		}

		break;
	}
	case GMSystemEventType::WindowSizeChanged:
	{
		updateScaling();
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
	const GMSystemMouseWheelEvent* wheelEvent = static_cast<const GMSystemMouseWheelEvent*>(event);
	// wheelStep为负数，表示滚轮向下滚动，为正数表示向上
	GMint32 wheelStep = wheelEvent->getDelta() / GM_WHEEL_DELTA * getScrollStep();
	return verticalScroll(wheelStep);
}

void GMWidget::onRenderTitle()
{
	D(d);
	GMRect rc = { 0, -d->titleHeight, d->width, d->titleHeight };
	drawSprite(GMControlPositionFlag::Fixed, d->titleStyle, rc, .99f);
	rc.x += d->titleOffset.x;
	rc.y += d->titleOffset.y;
	drawText(GMControlPositionFlag::Fixed, d->titleText, d->titleStyle, rc);
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

void GMWidget::calculateControlBoundingRect()
{
	D(d);
	// 重新计算所有控件的并集
	GMRect b = { 0 };
	for (auto c : d->controls)
	{
		if (c != d->borderControl)
			b = GM_unionRect(c->getBoundingRect(), b);
	}
	if (d->controlBoundingBox != b)
	{
		d->controlBoundingBox = b;
		updateVerticalScrollbar();
	}
}

void GMWidget::updateScaling()
{
	D(d);
	auto window = d->manager->getContext()->getWindow();
	GMRect fbrc = window->getFramebufferRect();
	GMRect winRc = window->getRenderRect();
	d->scaling[0] = static_cast<GMfloat>(winRc.width) / fbrc.width;
	d->scaling[1] = static_cast<GMfloat>(winRc.height) / fbrc.height;
}

void GMWidget::render(GMfloat elpasedTime)
{
	D(d);
	if (!d->borderControl)
		addBorder(d->areas[GMTextureArea::BorderArea].cornerRc);

	if (d->timeLastRefresh < s_timeRefresh)
	{
		d->timeLastRefresh = GM.getRunningStates().elapsedTime;
		refresh();
	}

	if (!d->visible ||
		(d->minimized && !d->title))
		return;

	// 更新内容区域以及滚动条状态
	calculateControlBoundingRect();
	if (needShowVerticalScrollbar())
		createVerticalScrollbar();
	else
		disableVerticalScrollbar();

	// 如果overflow样式为hidden，那么我们要在Widget的标题栏下方绘制一块模板
	// 这样，超出的部分将不会被绘制出来
	// 如果overflow样式为auto，那么我们不仅要在Widget的标题栏下方绘制一块模板，还需要多绘制一个滚动条
	// 如果overflow样式为visible，允许内容溢出边框
	GMRect contentRect = getContentRect();
	if (getOverflow() != GMOverflowStyle::Visible)
	{
		// 计算显示内容的矩形
		drawStencil(GMControlPositionFlag::Fixed, contentRect, .99f, false);
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

			// 如果overflow样式不为visible，我们只需要绘制内容区域里面的控件
			if (getOverflow() != GMOverflowStyle::Visible)
			{
				auto rc = control->getBoundingRect();
				if (control->getPositionFlag() == GMControlPositionFlag::Auto)
					rc.y += d->scrollOffsetY;
				if (GM_inRect(contentRect, GMPoint{ rc.x, rc.y }) || GM_inRect(contentRect, GMPoint{ rc.x + rc.width, rc.y + rc.height }))
					control->render(elpasedTime);
			}
		}

		if (d->focusControl && d->focusControl->getParent() == this)
			d->focusControl->render(elpasedTime);
	}

	if (getOverflow() != GMOverflowStyle::Visible)
		endStencil();

	// 最后绘制不随滚动状态而变化的部分，如边框，标题栏
	if (d->title)
		onRenderTitle();

	if (!d->minimized)
	{
		if (d->borderControl)
			d->borderControl->render(elpasedTime);
		if (d->verticalScrollbar)
			d->verticalScrollbar->render(elpasedTime);
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

GMControl* GMWidget::getControlAtPoint(const GMPoint& pt)
{
	D(d);
	GM_ASSERT(!FuzzyCompare(d->scaling[0], 0) && !FuzzyCompare(d->scaling[1], 0));

	// 响应scrollbars
	if (d->verticalScrollbar)
	{
		GM_ASSERT(d->verticalScrollbar->getPositionFlag() == GMControlPositionFlag::Fixed);
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

		GMPoint t = pt;
		if (control->getPositionFlag() == GMControlPositionFlag::Auto)
			t.y -= getScrollOffsetY(); //Auto的控件，要考虑widget滚动条

		if (control->getEnabled() && control->getVisible() && control->containsPoint(t))
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

void GMWidget::mapRect(GMControlPositionFlag positionFlag, GMRect& rc)
{
	D(d);
	rc.x += d->x;
	rc.y += d->y;

	// 如果是跟随滚动条，需要加上滚动条偏移
	if (positionFlag == GMControlPositionFlag::Auto)
		rc.y += d->scrollOffsetY;
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
	titleStyle.getFontColor().setCurrent(1.f);
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
	static const GMRect s_invalidRect = { 0 };
	if (!d->verticalScrollbar)
	{
		if (d->verticalScrollbarWidth == 0)
			gm_warning(gm_dbg_wrap("verticalScrollbarWidth is zero while rendering vertical scroll bar."));

		GMRect contentRect = getContentRect();
		d->verticalScrollbar.reset(GMControlScrollBar::createControl(
			this,
			d->width - d->borderMarginLeft - d->verticalScrollbarWidth,
			contentRect.y,
			d->verticalScrollbarWidth,
			contentRect.height,
			false
		));

		d->verticalScrollbar->setPositionFlag(GMControlPositionFlag::Fixed); // 不随Widget滚动条而移动
		connect(*d->verticalScrollbar, GM_SIGNAL(GMControlScrollBar, valueChanged), [=](auto sender, auto receiver) {
			this->verticalScrollTo(-static_cast<GMControlScrollBar*>(sender)->getValue());
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
	d->scrollOffsetY = 0;
}

bool GMWidget::needShowVerticalScrollbar()
{
	GMint32 overflowFlag = getContentOverflowFlag();
	return (overflowFlag & CanScrollUp || overflowFlag & CanScrollDown && getOverflow() != GMOverflowStyle::Visible) || getOverflow() == GMOverflowStyle::Scroll;
}

GMSystemMouseEvent* GMWidget::adjustMouseEvent(GMSystemMouseEvent* event, const GMControl* control)
{
	if (control->getPositionFlag() == GMControlPositionFlag::Auto)
	{
		GMPoint& pt = event->getPoint();
		pt.y -= getScrollOffsetY();
	}
	return event;
}

IWindow* GMWidget::getParentWindow()
{
	D(d);
	return d->parentWindow;
}

void GMWidget::setParentWindow(IWindow* window)
{
	D(d);
	d->parentWindow = window;
}

GMWidget* GMWidget::getNextCanvas()
{
	D(d);
	return d->nextWidget;
}

GMWidget* GMWidget::getPrevCanvas()
{
	D(d);
	return d->prevWidget;
}

const Vector<GMControl*>& GMWidget::getControls()
{
	D(d);
	return d->controls;
}

bool GMWidget::canKeyboardInput()
{
	D(d);
	return d->keyboardInput;
}

void GMWidget::setKeyboardInput(bool keyboardInput)
{
	D(d);
	d->keyboardInput = keyboardInput;
}

void GMWidget::setPosition(GMint32 x, GMint32 y)
{
	D(d);
	d->x = x;
	d->y = y;
}

void GMWidget::setBorderMargin(GMint32 left, GMint32 top)
{
	D(d);
	d->borderMarginLeft = left;
	d->borderMarginTop = top;
}

GMint32 GMWidget::getTitleHeight()
{
	D(d);
	return d->titleHeight;
}

GMWidgetResourceManager* GMWidget::getManager()
{
	D(d);
	return d->manager;
}

GMRect GMWidget::getSize()
{
	D(d);
	GMRect rc = { 0, 0, d->width, d->height };
	return rc;
}

void GMWidget::resetControlMouseOver()
{
	D(d);
	d->controlMouseOver = nullptr;
}

GMOverflowStyle GMWidget::getOverflow() GM_NOEXCEPT
{
	D(d);
	return d->overflow;
}

void GMWidget::setOverflow(GMOverflowStyle overflow)
{
	D(d);
	d->overflow = overflow;
	updateVerticalScrollbar();
}

GMint32 GMWidget::getContentPaddingLeft() GM_NOEXCEPT
{
	D(d);
	return d->contentPaddingLeft;
}

void GMWidget::setContentPaddingLeft(GMint32 padding)
{
	D(d);
	d->contentPaddingLeft = padding;
	updateVerticalScrollbar();
}

GMint32 GMWidget::getContentPaddingTop() GM_NOEXCEPT
{
	D(d);
	return d->contentPaddingTop;
}

void GMWidget::setContentPaddingTop(GMint32 padding)
{
	D(d);
	d->contentPaddingTop = padding;
	updateVerticalScrollbar();
}

GMint32 GMWidget::getContentPaddingRight() GM_NOEXCEPT
{
	D(d);
	return d->contentPaddingRight;
}

void GMWidget::setContentPaddingRight(GMint32 padding)
{
	D(d);
	d->contentPaddingRight = padding;
	updateVerticalScrollbar();
}

GMint32 GMWidget::getContentPaddingBottom() GM_NOEXCEPT
{
	D(d);
	return d->contentPaddingBottom;
}

void GMWidget::setContentPaddingBottom(GMint32 padding)
{
	D(d);
	d->contentPaddingBottom = padding;
	updateVerticalScrollbar();
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

END_NS