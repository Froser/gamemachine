#ifndef __SINGLEPRIMITIVEGAMEOBJECT_H__
#define __SINGLEPRIMITIVEGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
#include "rigidgameobject.h"
#include "gmdatacore/gmmap/gmmap.h"
BEGIN_NS

class SinglePrimitiveGameObject : public RigidGameObject
{
public:
	struct Size
	{
		GMfloat height;
		GMfloat radius;
		btVector3 halfExtents;
	};

	enum Type
	{
		Capsule,
		Cylinder,
		Cone,
	};

	static Type fromGMMapObjectType(GMMapObject::GMMapObjectType type);

public:
	SinglePrimitiveGameObject(Type type, const Size& size, Material& material);

public:
	virtual btCollisionShape* createCollisionShape() override;
	virtual void initPhysicsAfterCollisionObjectCreated() override;

private:
	void createTriangleMesh();

private:
	Type m_type;
	Material m_material;
	Size m_size;
};

END_NS
#endif