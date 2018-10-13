#ifndef __GMPHYSICSSHAPE_H__
#define __GMPHYSICSSHAPE_H__
#include <gmcommon.h>
#include "gmbulletforward.h"

struct GMVec3;
BEGIN_NS

GM_PRIVATE_OBJECT(GMPhysicsShape)
{
	btCollisionShape* shape = nullptr;
};

class GMPhysicsShape : public GMObject
{
	GM_DECLARE_PRIVATE(GMPhysicsShape);

	friend struct GMPhysicsShapeHelper;

private:
	GMPhysicsShape() = default;

public:
	~GMPhysicsShape();

public:
	void setShape(btCollisionShape* shape);
	btCollisionShape* getBulletShape();
	const btCollisionShape* getBulletShape() const;
};

struct GMPhysicsShapeHelper
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
		OUT GMModel** model
	);
};

END_NS
#endif