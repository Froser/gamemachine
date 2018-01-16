#ifndef __GMPHYSICSSHAPE_H__
#define __GMPHYSICSSHAPE_H__
#include <gmcommon.h>
#include "gmbulletforward.h"
#include "gmphysicsshape.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMPhysicsShape)
{
	btCollisionShape* shape = nullptr;
};

class GMPhysicsShape : public GMObject
{
	DECLARE_PRIVATE(GMPhysicsShape);

public:
	GMPhysicsShape() = default;
	~GMPhysicsShape();

public:
	void setShape(btCollisionShape* shape);
	btCollisionShape* getBulletShape();
	const btCollisionShape* getBulletShape() const;
};

struct GMPhysicsShapeCreator
{
	static void createBoxShape(const glm::vec3& halfExtents, OUT GMPhysicsShape** shape);
};

END_NS
#endif