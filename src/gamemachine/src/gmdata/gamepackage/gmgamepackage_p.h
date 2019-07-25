#ifndef __GAMEPACKAGE_P_H__
#define __GAMEPACKAGE_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGamePackage)
{
	GMString packagePath;
	GMScopedPtr<IGamePackageHandler> handler;
};

END_NS
#endif
