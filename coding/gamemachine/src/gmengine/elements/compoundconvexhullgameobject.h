#ifndef __COMPOUNDCONVEXHULLGAMEOBJECT_H__
#define __COMPOUNDCONVEXHULLGAMEOBJECT_H__
#include "common.h"
#include "convexhullgameobject.h"
#include <vector>
#include <map>
#include <string>
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
	std::multimap<std::string, btCollisionShape*> m_nameSet;
};

END_NS
#endif