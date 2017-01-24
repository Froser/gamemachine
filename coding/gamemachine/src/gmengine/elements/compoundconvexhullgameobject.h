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
	virtual GameObjectFindResults findChildObjectByName(const char* name) override;
	virtual btCollisionShape* createCollisionShape() override;
	virtual btTransform getCollisionShapeTransform(btCollisionShape* shape) override;
	virtual void removeShapeByName(const char* name) override;

private:
	std::vector<btConvexHullShape*> m_childs;
	std::multimap<std::string, std::pair<ChildObject*, btCollisionShape*>> m_nameSet;
};

END_NS
#endif