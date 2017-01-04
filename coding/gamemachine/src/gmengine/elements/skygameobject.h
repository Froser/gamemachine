#ifndef __SKYGAMEOBJECT_H__
#define __SKYGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"

class btDynamicsWorld;
class btCollisionShape;
BEGIN_NS

class SkyGameObject : public GameObject
{
public:
	SkyGameObject(GMfloat len, ITexture* skyTexture);

public:
	virtual void setWorld(GameWorld* world) override;
	virtual void setMass(btScalar) override;
	virtual void appendObjectToWorld(btDynamicsWorld* world) override;
	virtual btCollisionShape* createCollisionShape() override;

protected:
	virtual void getReadyForRender(DrawingList& list) override;

private:
	void createCoreObject();

private:
	GMfloat m_length;
	ITexture* m_texture;
};

END_NS
#endif