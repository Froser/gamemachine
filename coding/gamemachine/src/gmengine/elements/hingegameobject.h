#ifndef __HINGEGAMEOBJECT_H__
#define __HINGEGAMEOBJECT_H__
#include "common.h"
#include "convexhullgameobject.h"
BEGIN_NS

class HingeGameObject : public ConvexHullGameObject
{
public:
	HingeGameObject(Object* obj);
	~HingeGameObject();

public:
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;

private:
	AutoPtr<btHingeConstraint> m_constraint;
};

END_NS
#endif