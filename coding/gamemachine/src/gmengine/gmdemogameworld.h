#ifndef __GMDEMOGAMEWORLD_H__
#define __GMDEMOGAMEWORLD_H__
#include <gmcommon.h>
#include "gmgameworld.h"
#include "gmgameobject.h"
BEGIN_NS

// 提供一些示例的世界

GM_PRIVATE_OBJECT(GMDemoGameWorld)
{
	Map<GMString, GMGameObject*> renderList;
	Map<const GMGameObject*, GMString> renderListInv;
	Vector<GMGameObject*> gameObjects;
};

class GMDemoGameWorld : public GMGameWorld
{
	DECLARE_PRIVATE(GMDemoGameWorld)

	typedef GMGameWorld Base;

public:
	GMDemoGameWorld();

public:
	virtual void renderGameWorld() override;
	virtual GMPhysicsWorld* physicsWorld() override;

public:
	bool appendObject(const GMString& name, GMGameObject* obj);
	bool removeObject(const GMString& name);
	GMGameObject* findGameObject(const GMString& name);
	bool findGameObject(const GMGameObject* obj, REF GMString& name);

public:
	bool removeObject(GMGameObject* obj) override;
};

END_NS
#endif