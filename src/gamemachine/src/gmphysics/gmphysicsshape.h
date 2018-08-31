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

	friend struct GMPhysicsShapeCreator;

private:
	GMPhysicsShape() = default;

public:
	~GMPhysicsShape();

public:
	void setShape(btCollisionShape* shape);
	btCollisionShape* getBulletShape();
	const btCollisionShape* getBulletShape() const;
};

struct GMPhysicsShapeCreator
{
	static void createBoxShape(const GMVec3& halfExtents, REF GMPhysicsShapeAsset& physicsShape);
	static void createMeshFromShape(GMPhysicsShape* shape, GMGameObject* gameObject, GMGameWorld* world);
};

END_NS
#endif