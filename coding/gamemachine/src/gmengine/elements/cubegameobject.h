#ifndef __CUBEGAMEOBJECT_H__
#define __CUBEGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
BEGIN_NS

class CubeGameObject : public GameObject
{
public:
	CubeGameObject(const btVector3& extents, const Material eachMaterial[6]);
	CubeGameObject(const btVector3& extents, GMfloat magnification, const Material eachMaterial[6]);
	CubeGameObject(AUTORELEASE Object* obj);

public:
	void setExtents(const btVector3& extents);
	btVector3& getExtents();
	void setCollisionExtents(const btVector3& colHalfExtents);

public:
	virtual void appendObjectToWorld(btDynamicsWorld* world) override;

private:
	virtual btCollisionShape* createCollisionShape() override;

private:
	void createCoreObject();

private:
	btVector3 m_extents;
	Material m_eachMaterial[6];
	btVector3 m_collisionExtents;

	// 纹理的放大率
	GMfloat m_magnification;
};

END_NS
#endif