#ifndef __GMBULLETHELPER_H__
#define __GMBULLETHELPER_H__
#include <gmcommon.h>
#include "gmbulletforward.h"
#include <gmassets.h>
BEGIN_NS

class GMPhysicsShape;
struct GMBulletHelper
{
	static void createModelFromShape(
		GMPhysicsShape* shape,
		OUT GMModel** model);
};

END_NS
#endif