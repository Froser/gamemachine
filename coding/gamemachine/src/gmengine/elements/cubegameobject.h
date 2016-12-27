#ifndef __CUBEGAMEOBJECT_H__
#define __CUBEGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
BEGIN_NS

class CubeGameObject : public GameObject
{
public:
	CubeGameObject(const btVector3& extents, const btTransform& position, const Material eachMaterial[6]);

public:
	void setExtents(const btVector3& extents);
	btVector3& getExtents();

public:
	virtual void appendObjectToWorld(btDynamicsWorld* world) override;

private:
	virtual btCollisionShape* createCollisionShape() override;

private:
	void createCoreObject();

private:
	btVector3 m_extents;
	Material m_eachMaterial[6];
};

END_NS
#endif