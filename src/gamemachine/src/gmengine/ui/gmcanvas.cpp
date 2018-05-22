#include "stdafx.h"
#include "gmcanvas.h"
#include "gmcontrols.h"
#include "foundation/gamemachine.h"

GMCanvasResourceManager::~GMCanvasResourceManager()
{
	D(d);
	for (auto texture : d->textureCache)
	{
		GM_delete(texture);
	}
	GMClearSTLContainer(d->textureCache);
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

void GMCanvas::addControl(GMControl* control)
{
	D(d);
	d->controls.push_back(control);
	bool b = initControl(control);
	GM_ASSERT(b);
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
		if (elementHolder.type == control->getType())
			control->setElement(elementHolder.index, &elementHolder.element);
	}

	return control->onInit();
}