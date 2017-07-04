#ifndef __GMDEMOGAMEWORLD_H__
#define __GMDEMOGAMEWORLD_H__
#include "common.h"
#include "gmgameworld.h"
#include "foundation/vector.h"
#include "gmgameobject.h"
BEGIN_NS

// 提供一些示例的世界

GM_PRIVATE_OBJECT(GMDemoGameWorld)
{
	Map<GMString, GMGameObject*> renderList;
};

class GMDemoGameWorld : public GMGameWorld
{
	DECLARE_PRIVATE(GMDemoGameWorld)

public:
	GMDemoGameWorld();
	~GMDemoGameWorld();

public:
	virtual void renderGameWorld() override;
	virtual GMPhysicsWorld* physicsWorld() override;

public:
	bool appendObject(const GMString& name, GMGameObject* obj);
	void createCube(GMfloat extents[3], OUT GMGameObject** obj);
	GMGameObject* getGameObject(const GMString& name);
};

END_NS
#endif