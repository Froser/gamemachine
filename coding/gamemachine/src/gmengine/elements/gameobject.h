#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include "utilities/vector.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gameworld.h"
#include "gmdatacore/object.h"
#include "utilities/utilities.h"

BEGIN_NS
class GameWorld;
struct GameObjectPrivate
{
	GameObjectPrivate()
		: world(nullptr)
		, animationStartTick(0)
		, animationDuration(0)
		, id(0)
	{
	}

	GMuint id;
	AutoPtr<Object> object;
	GameWorld* world;
	GMint animationStartTick;
	GMint animationDuration;
};

class GameWorld;
class GameObject
{
	DEFINE_PRIVATE(GameObject)

public:
	GameObject(AUTORELEASE Object* obj);
	virtual ~GameObject() {};

public:
	void setObject(AUTORELEASE Object* obj);
	Object* getObject();

	virtual void setWorld(GameWorld* world);
	GameWorld* getWorld();

	void startAnimation(GMuint duration);
	void stopAnimation();

public:
	virtual void getReadyForRender(DrawingList& list);
	virtual void onAppendingObjectToWorld();
};

//GlyphObject
struct GlyphProperties
{
};

struct GlyphObjectPrivate
{
	std::wstring lastRenderText;
	std::wstring text;
	GlyphProperties properties;
	GMfloat left, bottom, width, height;
	ITexture* texture;
};

class Component;
class GlyphObject : public GameObject
{
	DEFINE_PRIVATE(GlyphObject)

public:
	GlyphObject();

public:
	void setText(const GMWChar* text);
	void setGeometry(GMfloat left, GMfloat bottom, GMfloat width, GMfloat height);
	void updateObject();

private:
	virtual void getReadyForRender(DrawingList& list) override;
	virtual void onAppendingObjectToWorld() override;

private:
	void constructObject();
};

//EntityObject
enum { EntityPlaneNum = 6 };

struct EntityObjectPrivate
{
	linear_math::Vector3 mins, maxs;
	Plane planes[EntityPlaneNum];
};

class EntityObject : public GameObject
{
	DEFINE_PRIVATE(EntityObject)

public:
	EntityObject(AUTORELEASE Object* obj);

public:
	Plane* getPlanes();
	void getBounds(REF linear_math::Vector3& mins, REF linear_math::Vector3& maxs);

private:
	void calc();
	void makePlanes();
};

END_NS
#endif
