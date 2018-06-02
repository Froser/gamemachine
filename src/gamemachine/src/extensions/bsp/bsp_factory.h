#ifndef __BSP_FACTORY_H__
#define __BSP_FACTORY_H__
#include <gmcommon.h>
BEGIN_NS

class GMBSPGameWorld;
struct GMBSPFactory
{
	static void createBSPGameWorld(const IRenderContext* context, const GMString& map, OUT GMBSPGameWorld** gameWorld);
};

END_NS
#endif