#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include "foundation/vector.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmdatacore/object.h"
#include "foundation/utilities/utilities.h"

BEGIN_NS
GM_PRIVATE_OBJECT(GMGameObject)
{
	GM_PRIVATE_CONSTRUCT(GMGameObject)
		: world(nullptr)
		, id(0)
	{
	}

	GMuint id;
	AutoPtr<Object> object;
	GMGameWorld* world;
};

class GMGameObject : public GMObject
{
	DECLARE_PRIVATE(GMGameObject)

public:
	GMGameObject(AUTORELEASE Object* obj);
	virtual ~GMGameObject() {};

public:
	void setObject(AUTORELEASE Object* obj);
	Object* getObject();

	virtual void setWorld(GMGameWorld* world);
	GMGameWorld* getWorld();

public:
	virtual void onAppendingObjectToWorld();
	virtual void onBeforeDraw();
};

//GMGlyphObject
struct GlyphProperties
{
};

GM_PRIVATE_OBJECT(GMGlyphObject)
{
	std::wstring lastRenderText;
	std::wstring text;
	GlyphProperties properties;
	GMfloat left, bottom, width, height;
	ITexture* texture;
};

class Component;
class GMGlyphObject : public GMGameObject
{
	DECLARE_PRIVATE(GMGlyphObject)

public:
	GMGlyphObject();

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

//GMEntityObject
enum { EntityPlaneNum = 6 };

GM_PRIVATE_OBJECT(GMEntityObject)
{
	linear_math::Vector3 mins, maxs;
	Plane planes[EntityPlaneNum];
};

class GMEntityObject : public GMGameObject
{
	DECLARE_PRIVATE(GMEntityObject)

public:
	GMEntityObject(AUTORELEASE Object* obj);

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

class SkyGameObject : public GMGameObject
{
	DECLARE_PRIVATE(SkyGameObject)

public:
	SkyGameObject(const Shader& shader, const linear_math::Vector3& min, const linear_math::Vector3& max);

private:
	void createSkyBox(OUT Object** obj);
};

END_NS
#endif
