#ifndef __GMLUAMETA_H__
#define __GMLUAMETA_H__

// {{BEGIN META INCLUDES}}
#include "meta/gamemachine_meta.h"
#include "meta/gmdebugger_meta.h"
#include "meta/ifactory_meta.h"
#include "meta/gmshaderhelper_meta.h"
#include "meta/gmmodelreader_meta.h"
#include "meta/gmgameworld_meta.h"
#include "meta/gmgameobject_meta.h"
#include "meta/gmskeletalgameobject_meta.h"
#include "meta/gmwidget_meta.h"
#include "meta/gmcontrol_meta.h"
// {{END META INCLUDES}}

BEGIN_NS
namespace luaapi
{
	void registerLib(GMLua* L)
	{
		// {{BEGIN META REGISTER}}
		GameMachine_Meta().registerFunctions(L);
		GMDebugger_Meta().registerFunctions(L);
		IFactory_Meta().registerFunctions(L);
		GMShaderHelper_Meta().registerFunctions(L);
		GMModelReader_Meta().registerFunctions(L);
		GMGameWorld_Meta().registerFunctions(L);
		GMGameObject_Meta().registerFunctions(L);
		GMSkeletalGameObject_Meta().registerFunctions(L);
		GMWidgetResourceManager_Meta().registerFunctions(L);
		GMUIConfiguration_Meta().registerFunctions(L);
		GMControlButton_Meta().registerFunctions(L);
		// {{END META REGISTER}}
	}
}
END_NS

#endif