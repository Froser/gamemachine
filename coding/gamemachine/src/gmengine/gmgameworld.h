#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include "common.h"
#include "foundation/vector.h"
#include "gmphysics/gmphysicsworld.h"
#include "gmdatacore/shader.h"

BEGIN_NS

class GMCharacter;
class ObjectPainter;

GM_PRIVATE_OBJECT(GMGameWorld)
{
	std::set<GMGameObject*> shapes;
	GMCharacter* character = nullptr;
	LightInfo ambientLight;
	bool start;
};

class GMGameWorld : public GMObject
{
	DECLARE_PRIVATE(GMGameWorld)

public:
	GMGameWorld();
	virtual ~GMGameWorld();

public:
	virtual void renderGameWorld() = 0;
	virtual GMPhysicsWorld* physicsWorld() = 0;

public:
	void initialize();
	void appendObjectAndInit(AUTORELEASE GMGameObject* obj);
	void simulateGameWorld();
	void setDefaultAmbientLight(const LightInfo& lightInfo);
	LightInfo& getDefaultAmbientLight();

// characters:
public:
	virtual void setMajorCharacter(GMCharacter* character);
	GMCharacter* getMajorCharacter();

private:
	ObjectPainter* createPainterForObject(GMGameObject* obj);
};

END_NS
#endif