#ifndef __HALLUCINATIONGAMEOBJECT_H__
#define __HALLUCINATIONGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
BEGIN_NS

class HallucinationGameObject : public GameObject
{
public:
	HallucinationGameObject(AUTORELEASE Object* obj);

public:
	virtual void setLocalScaling(const btVector3& scale) override;
	virtual void initPhysics(btDynamicsWorld* world) override;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;
	virtual btCollisionShape* createCollisionShape() override;
	virtual void getReadyForRender(DrawingList& list) override;
	virtual btCollisionObject* createCollisionObject() override;

protected:
	const btVector3& getLocalScaling();

private:
	btVector3 m_scale;
};

END_NS
#endif