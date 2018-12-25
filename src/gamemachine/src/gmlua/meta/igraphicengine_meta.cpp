#include "stdafx.h"
#include "igraphicengine_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

#define NAME "Template"

using namespace gm::luaapi;

IGraphicEngineProxy::IGraphicEngineProxy(IGraphicEngine* engine /*= nullptr*/)
{
	D(d);
	d->engine = engine;
}

bool IGraphicEngineProxy::registerMeta()
{
	GM_META(engine);
	return true;
}
