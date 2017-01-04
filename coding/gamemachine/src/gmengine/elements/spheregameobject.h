#ifndef __SPHEREGAMEOBJECT_H__
#define __SPHEREGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
BEGIN_NS

class SphereGameObject : public GameObject
{
public:
	SphereGameObject(GMfloat radius, GMfloat slices, GMfloat stacks, const Material& material);

public:
	virtual void appendObjectToWorld(btDynamicsWorld* world) override;

private:
	virtual btCollisionShape* createCollisionShape() override;

private:
	void createCoreObject();

private:
	GMfloat m_radius;
	GMfloat m_slices;
	GMfloat m_stacks;
	Material m_material;
};


END_NS
#endif