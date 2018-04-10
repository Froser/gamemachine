#ifndef __GMDEMOGAMEWORLD_H__
#define __GMDEMOGAMEWORLD_H__
#include <gmcommon.h>
#include "gmgameworld.h"
#include "gameobjects/gmgameobject.h"
BEGIN_NS

// 提供一些示例的世界

GM_PRIVATE_OBJECT(GMDemoGameWorld)
{
	HashMap<GMString, GMGameObject*, GMStringHashFunctor> renderList;
	Map<const GMGameObject*, GMString> renderListInv;
	Vector<GMGameObject*> gameObjects;
};

class GMDemoGameWorld : public GMGameWorld
{
	DECLARE_PRIVATE_AND_BASE(GMDemoGameWorld, GMGameWorld)

public:
	GMDemoGameWorld();
	~GMDemoGameWorld();

public:
	virtual void renderScene() override;

public:
	virtual bool removeObject(GMGameObject* obj) override;

public:
	bool addObject(const GMString& name, GMGameObject* obj);
	bool removeObject(const GMString& name);
	GMGameObject* findGameObject(const GMString& name);
	bool findGameObject(const GMGameObject* obj, REF GMString& name);

private:
	// 这个应该由appendObject来转调，appendObject会传入一个对象的名称
	using GMGameWorld::addObjectAndInit;
};

END_NS
#endif
