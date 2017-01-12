#ifndef __SKYGAMEOBJECT_H__
#define __SKYGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "hallucinationgameobject.h"

class btDynamicsWorld;
class btCollisionShape;
BEGIN_NS

class SkyGameObject : public HallucinationGameObject
{
public:
	SkyGameObject(GMfloat len, ITexture* skyTexture);

public:
	virtual void setWorld(GameWorld* world) override;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;

protected:
	virtual void getReadyForRender(DrawingList& list) override;

private:
	void initCoreObject();

private:
	GMfloat m_length;
	ITexture* m_texture;
};

END_NS
#endif