#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include "foundation/vector.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gameworld.h"
#include "gmdatacore/object.h"
#include "foundation/utilities/utilities.h"

BEGIN_NS
GM_PRIVATE_OBJECT(GameObject)
{
	GM_PRIVATE_CONSTRUCT(GameObject)
		: world(nullptr)
		, id(0)
	{
	}

	GMuint id;
	AutoPtr<Object> object;
	GameWorld* world;
};

class GameObject : public GMObject
{
	DECLARE_PRIVATE(GameObject)

public:
	GameObject(AUTORELEASE Object* obj);
	virtual ~GameObject() {};

public:
	void setObject(AUTORELEASE Object* obj);
	Object* getObject();

	virtual void setWorld(GameWorld* world);
	GameWorld* getWorld();

public:
	virtual void onAppendingObjectToWorld();
	virtual void onBeforeDraw();
};

//GlyphObject
struct GlyphProperties
{
};

GM_PRIVATE_OBJECT(GlyphObject)
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
	DECLARE_PRIVATE(GlyphObject)

public:
	GlyphObject();

public:
	void setText(const GMWChar* text);
	void setGeometry(GMfloat left, GMfloat bottom, GMfloat width, GMfloat height);
	void updateObject();

private:
	virtual void onBeforeDraw() override;
	virtual void onAppendingObjectToWorld() override;

private:
	void constructObject();
};

//EntityObject
enum { EntityPlaneNum = 6 };

GM_PRIVATE_OBJECT(EntityObject)
{
	linear_math::Vector3 mins, maxs;
	Plane planes[EntityPlaneNum];
};

class EntityObject : public GameObject
{
	DECLARE_PRIVATE(EntityObject)

public:
	EntityObject(AUTORELEASE Object* obj);

public:
	Plane* getPlanes();
	void getBounds(REF linear_math::Vector3& mins, REF linear_math::Vector3& maxs);

private:
	void calc();
	void makePlanes();
};

// SkyObject
GM_PRIVATE_OBJECT(SkyGameObject)
{
	linear_math::Vector3 min;
	linear_math::Vector3 max;
	Shader shader;
};

class SkyGameObject : public GameObject
{
	DECLARE_PRIVATE(SkyGameObject)

public:
	SkyGameObject(const Shader& shader, const linear_math::Vector3& min, const linear_math::Vector3& max);

private:
	void createSkyBox(OUT Object** obj);
};

END_NS
#endif
