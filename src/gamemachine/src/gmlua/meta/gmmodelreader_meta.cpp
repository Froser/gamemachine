#include "stdafx.h"
#include "gmmodelreader_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include <gmmodelreader.h>
#include "gmasset_meta.h"
#include "irendercontext_meta.h"

#define NAME "GMModelReader"

using namespace gm::luaapi;

GM_PRIVATE_OBJECT(GMModelLoadSettingsProxy)
{
	GMString filename;
	GMString directory;
	GMObject* context = nullptr;
	IRenderContextProxy ref;
	GMModelPathType type;
};

class GMModelLoadSettingsProxy : public GMObject
{
	GM_DECLARE_PRIVATE(GMModelLoadSettingsProxy)

public:
	GMModelLoadSettingsProxy();

public:
	virtual bool registerMeta() override;

public:
	GMModelLoadSettings toModelLoadSettings();
};

GMModelLoadSettingsProxy::GMModelLoadSettingsProxy()
{
	D(d);
	d->context = &d->ref;
}

bool GMModelLoadSettingsProxy::registerMeta()
{
	GM_META(filename);
	GM_META(directory);
	GM_META(context);
	GM_META_WITH_TYPE(type, GMMetaMemberType::Int);
	return true;
}


GMModelLoadSettings GMModelLoadSettingsProxy::toModelLoadSettings()
{
	D(d);
	return GMModelLoadSettings(d->filename, d->ref.get(), d->type, d->directory);
}

namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_FUNC(load)
	{
		static const GMString s_invoker = NAME ".load";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".load");
		GMModelLoadSettingsProxy settings;
		GMArgumentHelper::popArgumentAsObject(L, settings, s_invoker);
		GMSceneAsset asset;
		GMModelReader::load(settings.toModelLoadSettings(), GMModelReader::Assimp, asset);
		GMAssetProxy proxy(asset);
		return GMReturnValues(L, proxy);
	}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(load),
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

GM_LUA_REGISTER_IMPL(GMModelReader_Meta, NAME, g_meta);