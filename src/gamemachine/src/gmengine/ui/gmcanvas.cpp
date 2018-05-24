#include "stdafx.h"
#include "gmcanvas.h"
#include "gmcontrols.h"
#include "foundation/gamemachine.h"
#include "../gameobjects/gmgameobject.h"
#include "../gameobjects/gm2dgameobject.h"
#include "../gmtypoengine.h"
#include "gmdata/gmmodel.h"
#include "gmdata/glyph/gmglyphmanager.h"

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
}

GMCanvasResourceManager::~GMCanvasResourceManager()
{
	D(d);
	for (auto texture : d->textureCache)
	{
		GM_delete(texture);
	}
	GM_delete(d->textObject);

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

void GMCanvasResourceManager::addTexture(ITexture* texture)
{
	D(d);
	d->textureCache.push_back(texture);
}

ITexture* GMCanvasResourceManager::getTexture(size_t i)
{
	D(d);
	return d->textureCache[i];
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
	initDefaultElements();
}

GMCanvas::~GMCanvas()
{
	D(d);
	for (auto elementHolder : d->defaultElements)
	{
		GM_delete(elementHolder);
	}
	removeAllControls();
}

void GMCanvas::addControl(GMControl* control)
{
	D(d);
	d->controls.push_back(control);
	bool b = initControl(control);
	GM_ASSERT(b);
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

void GMCanvas::drawText(
	const GMString& text,
	GMElement* element,
	const GMRect& rcDest,
	bool bShadow,
	GMint nCount,
	bool bCenter
)
{
	D(d);
	// 不需要绘制透明元素
	if (element->getFontColor().getCurrent().getW() == 0)
		return;

	// TODO 先不考虑阴影什么的
	const GMVec4& fontColor = element->getFontColor().getCurrent();

	GMTextGameObject* textObject = d->manager->getTextObject();
	textObject->setText(text);
	textObject->setGeometry(rcDest);
	textObject->draw();
}

void GMCanvas::initDefaultElements()
{
	GMElement element;

	// Static
	element.setFont(0);

	GMElementBlendColor blendColor;
	blendColor.getStates()[(GMuint)GMControlState::Disabled] = GMVec4(.87f, .87f, .87f, .87f);
	element.setFontColor(blendColor);

	setDefaultElement(GMControlType::Static, 0, &element);
}

void GMCanvas::setDefaultElement(GMControlType type, GMuint index, GMElement* element)
{
	D(d);
	for (auto elementHolder : d->defaultElements)
	{
		if (elementHolder->type == type && elementHolder->index == index)
		{
			elementHolder->element = *element;
			return;
		}
	}

	GMElementHolder* elementHolder = new GMElementHolder();
	elementHolder->type = type;
	elementHolder->index = index;
	elementHolder->element = *element;
	d->defaultElements.push_back(elementHolder);
}

bool GMCanvas::initControl(GMControl* control)
{
	D(d);
	GM_ASSERT(control);
	if (!control)
		return false;

	control->setIndex(d->controls.size());
	for (auto& elementHolder : d->defaultElements)
	{
		if (elementHolder->type == control->getType())
			control->setElement(elementHolder->index, &elementHolder->element);
	}

	return control->onInit();
}

void GMCanvas::render(GMfloat elpasedTime)
{
	D(d);
	if (d->timeLastRefresh < s_timeRefresh)
	{
		d->timeLastRefresh = GM.getGameTimeSeconds();
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

void GMCanvas::refresh()
{
	D(d);
	if (s_controlFocus)
		s_controlFocus->onFocusOut();

	if (d->controlMouseOver)
		d->controlMouseOver->onMouseLeave();

	s_controlFocus = nullptr;
	s_controlPressed = nullptr;
	d->controlMouseOver = NULL;

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

void GMCanvas::clearFocus()
{
	if (s_controlFocus)
	{
		s_controlFocus->onFocusOut();
		s_controlFocus = nullptr;
	}

	ReleaseCapture(); //TODO WINAPI
}