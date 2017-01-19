#ifndef __COMPOUNDCONVEXHULLGAMEOBJECT_H__
#define __COMPOUNDCONVEXHULLGAMEOBJECT_H__
#include "common.h"
#include "convexhullgameobject.h"
#include <vector>
BEGIN_NS

class CompoundConvexHullGameObject : public ConvexHullGameObject
{
public:
	CompoundConvexHullGameObject(AUTORELEASE Object* object);
	~CompoundConvexHullGameObject();

private:
	virtual btCollisionShape* createCollisionShape() override;

private:
	std::vector<btConvexHullShape*> m_childs;
};

END_NS
#endif