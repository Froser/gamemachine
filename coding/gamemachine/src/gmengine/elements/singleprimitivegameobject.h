#ifndef __SINGLEPRIMITIVEGAMEOBJECT_H__
#define __SINGLEPRIMITIVEGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
#include "rigidgameobject.h"
BEGIN_NS

class SinglePrimitiveGameObject : public RigidGameObject
{
public:
	enum Type
	{
		Sphere,
	};

public:
	SinglePrimitiveGameObject(Type type, GMfloat radius, Material& material);

public:
	virtual btCollisionShape* createCollisionShape() override;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;

private:
	void createMesh();

private:
	Type m_type;
	Material m_material;
	GMfloat m_radius;
};

END_NS
#endif