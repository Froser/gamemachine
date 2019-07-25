#ifndef __GMPHYSICSSHAPE_H__
#define __GMPHYSICSSHAPE_H__
#include <gmcommon.h>
#include "gmbulletforward.h"
#include <gmassets.h>

struct GMVec3;
BEGIN_NS

GM_PRIVATE_CLASS(GMPhysicsShape);
class GM_EXPORT GMPhysicsShape
{
	GM_DECLARE_PRIVATE(GMPhysicsShape);
	GM_DISABLE_COPY_ASSIGN(GMPhysicsShape)
	GM_DECLARE_PROPERTY(GMModelAsset, ModelCache)
	GM_FRIEND_STRUCT(GMPhysicsShapeHelper);

private:
	GMPhysicsShape();

public:
	~GMPhysicsShape();

public:
	void setShape(btCollisionShape* shape);
	btCollisionShape* getBulletShape();
	const btCollisionShape* getBulletShape() const;
};

struct GM_EXPORT GMPhysicsShapeHelper
{
	static void createCubeShape(const GMVec3& halfExtents, REF GMPhysicsShapeAsset& physicsShape);
	static void createCylinderShape(const GMVec3& halfExtents, REF GMPhysicsShapeAsset& physicsShape);
	static void createConeShape(GMfloat radius, GMfloat height, REF GMPhysicsShapeAsset& physicsShape);
	static void createSphereShape(GMfloat radius, REF GMPhysicsShapeAsset& physicsShape);
	static bool createConvexShapeFromTriangleModel(
		GMModelAsset model,
		REF GMPhysicsShapeAsset& physicsShape,
		bool optimizeConvex = false,
		const GMVec3& scaling = GMVec3(1, 1, 1)
	);
	static void createModelFromShape(
		GMPhysicsShape* shape,
		REF GMModelAsset& asset
	);
};

END_NS
#endif