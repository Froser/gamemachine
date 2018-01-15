#ifndef __GMBULLETHELPER_H__
#define __GMBULLETHELPER_H__
#include <gmcommon.h>
#include "gmbulletforward.h"
#include "gmbulletincludes.h"
BEGIN_NS

struct GMBulletHelper
{
	static void collisionShape2TriangleMesh(
		btCollisionShape* collisionShape,
		const btTransform& parentTransform,
		btAlignedObjectArray<btVector3>& vertexPositions,
		btAlignedObjectArray<btVector3>& vertexNormals,
		btAlignedObjectArray<int>& indicesOut);
};

END_NS
#endif