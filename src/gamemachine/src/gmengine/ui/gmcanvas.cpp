#include "stdafx.h"
#include "gmcanvas.h"
#include "gmcontrols.h"
#include "foundation/gamemachine.h"
#include "../gameobjects/gmgameobject.h"
#include "../gameobjects/gm2dgameobject.h"
#include "../gmtypoengine.h"
#include "gmdata/gmmodel.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "foundation/gmmessage.h"

namespace
{
	GMControl* getNextControl(GMControl* control)
	{
		GMCanvas* parentCanvas = control->getParent();
		GMuint index = control->getIndex() + 1;

		// 如果下一个控件不在此画布内，则跳到下一个画布进行查找
		while (index >= (GMuint)parentCanvas->getControls().size())
		{
			parentCanvas = parentCanvas->getNextCanvas();
			index = 0;
		}

		return parentCanvas->getControls()[index];
	}

	GMControl* getPrevControl(GMControl* control)
	{
		GMCanvas* parentCanvas = control->getParent();
		GMuint index = control->getIndex() - 1;
		while (index < 0)
		{
			parentCanvas = parentCanvas->getPrevCanvas();
			if (!parentCanvas)
				parentCanvas = control->getParent();
			index = parentCanvas->getControls().size() - 1;
		}
		return parentCanvas->getControls()[index];
	}

	GMsize_t indexOf(const Vector<GMCanvas*>& canvases, GMCanvas* targetCanvas)
	{
		for (GMsize_t i = 0; i < canvases.size(); ++i)
		{
			if (canvases[i] == targetCanvas)
				return i;
		}
		return -1;
	}
}

GMCanvasResourceManager::GMCanvasResourceManager()
{
	D(d);
	d->screenQuadModel = new GMModel();
	d->screenQuadModel->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);
	d->screenQuadModel->setUsageHint(GMUsageHint::DynamicDraw);
	d->screenQuadModel->setType(GMModelType::Model2D);
	GMMesh* mesh = new GMMesh(d->screenQuadModel);

	// 增加4个空白顶点
	mesh->vertex(GMVertex());
	mesh->vertex(GMVertex());
	mesh->vertex(GMVertex());
	mesh->vertex(GMVertex());

	d->screenQuad = new GMGameObject(GMAssets::createIsolatedAsset(GMAssetType::Model, d->screenQuadModel));
	GM.createModelDataProxyAndTransfer(d->screenQuadModel);

	d->textObject = new GMTextGameObject();
	d->spriteObject = new GMSprite2DGameObject();
}

GMCanvasResourceManager::~GMCanvasResourceManager()
{
	D(d);
	for (auto textureInfo : d->textureCache)
	{
		GM_delete(textureInfo.texture);
	}
	GM_delete(d->textObject);
	GM_delete(d->spriteObject);

	GM_delete(d->screenQuad);
	GM_delete(d->screenQuadModel);
}

GMModel* GMCanvasResourceManager::getScreenQuadModel()
{
	D(d);
	return d->screenQuadModel;
}

GMGameObject* GMCanvasResourceManager::getScreenQuad()
{
	D(d);
	return d->screenQuad;
}

GMsize_t GMCanvasResourceManager::addTexture(ITexture* texture, GMint width, GMint height)
{
	D(d);
	GMCanvasTextureInfo texInfo;
	texInfo.texture = texture;
	texInfo.width = width;
	texInfo.height = height;
	d->textureCache.push_back(texInfo);
	return d->textureCache.size() - 1;
}

void GMCanvasResourceManager::registerCanvas(GMCanvas* canvas)
{
	D(d);
	for (auto c : d->canvases)
	{
		if (c == canvas)
			return;
	}

	// 将Canvas设置成一个环
	GMsize_t sz = d->canvases.size();
	d->canvases.push_back(canvas);
	if (sz > 1)
		d->canvases[sz - 2]->setNextCanvas(canvas);
	d->canvases[sz - 1]->setNextCanvas(d->canvases[0]);
}

const GMCanvasTextureInfo& GMCanvasResourceManager::getTexture(GMsize_t index)
{
	D(d);
	return d->textureCache[index];
}

void GMCanvasResourceManager::onRenderRectResized()
{
	D(d);
	const GMGameMachineRunningStates& runningStates = GM.getGameMachineRunningStates();
	d->backBufferWidth = runningStates.renderRect.width;
	d->backBufferHeight = runningStates.renderRect.height;
}

GMfloat GMCanvas::s_timeRefresh = 0;
GMControl* GMCanvas::s_controlFocus = nullptr;
GMControl* GMCanvas::s_controlPressed = nullptr;

GMCanvas::GMCanvas(GMCanvasResourceManager* manager)
{
	D(d);
	d->manager = manager;
	d->nextCanvas = d->prevCanvas = this;
}

GMCanvas::~GMCanvas()
{
	removeAllControls();
}

void GMCanvas::addControl(GMControl* control)
{
	D(d);
	d->controls.push_back(control);
	bool b = initControl(control);
	GM_ASSERT(b);
}

const GMRect& GMCanvas::getArea(GMCanvasControlArea::Area area)
{
	D(d);
	return d->areas[area];
}

void GMCanvas::addStatic(
	GMint id,
	const GMString& text,
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	bool isDefault,
	OUT GMControlStatic** out
)
{
	GMControlStatic* staticControl = new GMControlStatic(this);
	if (out)
		*out = staticControl;

	addControl(staticControl);
	staticControl->setId(id);
	staticControl->setText(text);
	staticControl->setPosition(x, y);
	staticControl->setSize(width, height);
	staticControl->setIsDefault(isDefault);
}

void GMCanvas::addButton(
	GMint id,
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
	buttonControl->setId(id);
	buttonControl->setText(text);
	buttonControl->setPosition(x, y);
	buttonControl->setSize(width, height);
	buttonControl->setIsDefault(isDefault);
}

void GMCanvas::drawText(
	const GMString& text,
	GMStyle& style,
	const GMRect& rc,
	bool shadow,
	bool center
)
{
	// 不需要绘制透明元素
	if (style.getFontColor().getCurrent().getW() == 0)
		return;

	D(d);
	// TODO 先不考虑阴影什么的
	const GMVec4& fontColor = style.getFontColor().getCurrent();
	GMTextGameObject* textObject = d->manager->getTextObject();
	textObject->setColorType(Plain);
	textObject->setColor(fontColor);
	textObject->setText(text);
	textObject->setGeometry(rc);
	textObject->setCenter(center);
	textObject->draw();
}

void GMCanvas::drawSprite(
	GMStyle& style,
	const GMRect& rc,
	GMfloat depth
)
{
	// 不需要绘制透明元素
	if (style.getFontColor().getCurrent().getW() == 0)
		return;

	D(d);
	// TODO Caption
	const GMRect& textureRc = style.getTextureRect();
	GMuint texId = style.getTexture();
	const GMCanvasTextureInfo& texInfo = d->manager->getTexture(texId);

	GMSprite2DGameObject* spriteObject = d->manager->getSpriteObject();
	spriteObject->setDepth(depth);
	spriteObject->setGeometry(rc);
	spriteObject->setTexture(texInfo.texture);
	spriteObject->setTextureRect(textureRc);
	spriteObject->setTextureSize(texInfo.width, texInfo.height);
	spriteObject->setColor(style.getTextureColor().getCurrent());
	spriteObject->draw();
}

void GMCanvas::requestFocus(GMControl* control)
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

bool GMCanvas::initControl(GMControl* control)
{
	D(d);
	GM_ASSERT(control);
	if (!control)
		return false;

	control->setIndex(d->controls.size());
	control->initStyles();
	return control->onInit();
}

void GMCanvas::setPrevCanvas(GMCanvas* prevCanvas)
{
	D(d);
	d->prevCanvas = prevCanvas;
}

void GMCanvas::addArea(GMCanvasControlArea::Area area, const GMRect& rc)
{
	D(d);
	d->areas[area] = rc;
}

bool GMCanvas::msgProc(GMSystemEvent* event)
{
	D(d);
	GMSystemEventType type = event->getType();
	switch (type)
	{
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
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDblClick:
	case GMSystemEventType::MouseWheel:
	{
		GMSystemMouseEvent* mouseEvent = gm_cast<GMSystemMouseEvent*>(event);
		GMSystemMouseEvent cacheEvent = *mouseEvent;
		GMPoint pt = mouseEvent->getPoint();
		pt.x -= d->x;
		pt.y -= d->y;
		cacheEvent.setPoint(pt);
		if (s_controlFocus &&
			s_controlFocus->getParent() == this &&
			s_controlFocus->getEnabled())
		{
			if (s_controlFocus->handleMouse(&cacheEvent))
				return true;
		}

		// 点击测试，找到鼠标所在位置的控件
		GMControl* control = getControlAtPoint(pt);
		if (control && control->getEnabled())
		{
			if (control->handleMouse(mouseEvent))
				return true;
		}
		else
		{
			// 如果没有找到任何控件，那么当前的焦点控件失去焦点
			if (type == GMSystemEventType::KeyDown && 
				(mouseEvent->getButton() & GMMouseButton_Left) != 0 &&
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

void GMCanvas::render(GMfloat elpasedTime)
{
	D(d);
	if (d->timeLastRefresh < s_timeRefresh)
	{
		d->timeLastRefresh = GM.getGameMachineRunningStates().elapsedTime;
		refresh();
	}

	if (!d->visible ||
		(d->minimized && !d->caption))
		return;

	bool backgroundVisible =
		d->colorTopLeft[2] > 0 ||
		d->colorTopRight[2] > 0 ||
		d->colorBottomLeft[2] > 0 ||
		d->colorBottomRight[2] > 0;

	if (!d->minimized && backgroundVisible)
	{
		GMfloat left, right, top, bottom;
		left = d->x * 2.0f / d->manager->getBackBufferWidth() - 1.0f;
		right = (d->x + d->width) * 2.0f / d->manager->getBackBufferWidth() - 1.0f;
		top = 1.0f - d->y * 2.0f / d->manager->getBackBufferHeight();
		bottom = 1.0f - (d->y + d->height) * 2.0f / d->manager->getBackBufferHeight();

		GMVertex vertices[4] = {
			{ { left,  top,    .5f }, { 0 }, { 0, 0 }, { 0 }, { 0 }, { 0 }, { d->colorTopLeft[0],     d->colorTopLeft[1],     d->colorTopLeft[2],     d->colorTopLeft[3]     } },
			{ { right, top,    .5f }, { 0 }, { 1, 0 }, { 0 }, { 0 }, { 0 }, { d->colorTopRight[0],    d->colorTopRight[1],    d->colorTopRight[2],    d->colorTopRight[3]    } },
			{ { left,  bottom, .5f }, { 0 }, { 0, 1 }, { 0 }, { 0 }, { 0 }, { d->colorBottomLeft[0],  d->colorBottomLeft[1],  d->colorBottomLeft[2],  d->colorBottomLeft[3]  } },
			{ { right, bottom, .5f }, { 0 }, { 1, 1 }, { 0 }, { 0 }, { 0 }, { d->colorBottomRight[0], d->colorBottomRight[1], d->colorBottomRight[2], d->colorBottomRight[3] } },
		};

		GMModel* quad = d->manager->getScreenQuadModel();
		GMModelDataProxy* proxy = quad->getModelDataProxy();
		// 使用proxy来更新其顶点
		proxy->beginUpdateBuffer();
		void* buffer = proxy->getBuffer();
		GM_ASSERT(buffer);
		memcpy_s(buffer, sizeof(vertices), &vertices, sizeof(vertices));
		proxy->endUpdateBuffer();

		// 开始绘制背景
		d->manager->getScreenQuad()->draw();
	}

	// TODO getTextureNode
	// TODO caption

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

void GMCanvas::setNextCanvas(GMCanvas* nextCanvas)
{
	D(d);
	if (!nextCanvas)
		nextCanvas = this;
	d->nextCanvas = nextCanvas;
	if (nextCanvas)
		nextCanvas->setPrevCanvas(this);
}

void GMCanvas::refresh()
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

void GMCanvas::focusDefaultControl()
{
	D(d);
	for (auto& control : d->controls)
	{
		if (control->isDefault())
		{
			clearFocus();

			s_controlFocus = control;
			s_controlFocus->onFocusIn();
			return;
		}
	}
}

void GMCanvas::removeAllControls()
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

GMControl* GMCanvas::getControlAtPoint(const GMPoint& pt)
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

bool GMCanvas::onCycleFocus(bool goForward)
{
	D(d);
	GMControl* control = nullptr;
	GMCanvas* canvas = nullptr;
	GMCanvas* lastCanvas = nullptr;
	const Vector<GMCanvas*>& canvases = d->manager->getCanvases();
	GMint sz = (GMint)canvases.size();

	if (!s_controlFocus)
	{
		if (goForward)
		{
			for (GMint i = 0; i < sz; ++i)
			{
				canvas = lastCanvas = canvases[i];
				const Vector<GMControl*> controls = canvas->getControls();
				if (canvas && controls.size() > 0)
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
				canvas = lastCanvas = canvases[i];
				const Vector<GMControl*> controls = canvas->getControls();
				if (canvas && controls.size() > 0)
				{
					control = controls[controls.size() - 1];
					break;
				}
			}
		}

		if (!canvas || !control)
			return true;
	}
	else if (s_controlFocus->getParent() != this)
	{
		// 当前获得焦点的控件属于另外一个canvas，所以要交给它来处理
		return false;
	}
	else
	{
		lastCanvas = s_controlFocus->getParent();
		control = (goForward) ? getNextControl(s_controlFocus) : getNextControl(s_controlFocus);
		canvas = control->getParent();
	}

	while (true)
	{
		// 如果我们转了一圈回来，那么我们不会设置任何焦点了。
		const Vector<GMCanvas*> canvases = d->manager->getCanvases();
		GMsize_t lastCanvasIndex = indexOf(canvases, lastCanvas);
		GMsize_t canvasIndex = indexOf(canvases, canvas);
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

		lastCanvas = canvas;
		control = (goForward) ? getNextControl(control) : getPrevControl(control);
		canvas = control->getParent();
	}

	// 永远都不会到这里来，因为canvas是个环，只会在上面return
	GM_ASSERT(false);
	return false;
}

void GMCanvas::onMouseMove(const GMPoint& pt)
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

void GMCanvas::clearFocus()
{
	if (s_controlFocus)
	{
		s_controlFocus->onFocusOut();
		s_controlFocus = nullptr;
	}

	// TODO ReleaseCapture()
}