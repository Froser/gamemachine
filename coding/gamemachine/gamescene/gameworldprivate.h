#ifndef __GAMEWORLD_PRIVATE_H__
#define __GAMEWORLD_PRIVATE_H__
#include "common.h"
#include "btBulletDynamicsCommon.h"
BEGIN_NS

class GameWorldPrivate
{
private:
	GameWorldPrivate();

	void setGravity(GMfloat x, GMfloat y, GMfloat z);

private:
	btVector3 m_gravity;
};

END_NS
#endif