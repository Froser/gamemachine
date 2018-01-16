#ifndef __GMBULLETHELPER_H__
#define __GMBULLETHELPER_H__
#include <gmcommon.h>
#include "gmbulletforward.h"
#include <gmassets.h>
BEGIN_NS

class GMPhysicsShape;
struct GMBulletHelper
{
	static GMAsset createModelFromShape(
		GMPhysicsShape* shape);
};

END_NS
#endif