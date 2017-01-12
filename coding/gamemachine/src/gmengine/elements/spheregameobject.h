#ifndef __SPHEREGAMEOBJECT_H__
#define __SPHEREGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
#include "rigidgameobject.h"
BEGIN_NS

class SphereGameObject : public RigidGameObject
{
public:
	SphereGameObject(GMfloat radius, GMfloat slices, GMfloat stacks, const Material& material);
	SphereGameObject(GMfloat radius, AUTORELEASE Object* obj);

private:
	virtual btCollisionShape* createCollisionShape() override;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;

private:
	void initCoreObject();

private:
	GMfloat m_radius;
	GMfloat m_slices;
	GMfloat m_stacks;
	Material m_material;
};


END_NS
#endif