#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include <gmcommon.h>
#include <utilities.h>
#include <gmmodel.h>

BEGIN_NS

typedef void(*GMGameObjectDestructor)(GMGameObject*);

GM_PRIVATE_OBJECT(GMGameObject)
{
	GMuint id = 0;
	GMGameWorld* world = nullptr;
	GMModel* model = nullptr;
	GMGameObjectDestructor destructor = nullptr;
	linear_math::Matrix4x4 scaling = linear_math::Matrix4x4::identity();
	linear_math::Matrix4x4 translation = linear_math::Matrix4x4::identity();
	linear_math::Quaternion rotation = linear_math::Quaternion::identity();
	linear_math::Matrix4x4 transformMatrix = linear_math::Matrix4x4::identity();

	bool canDeferredRendering = true;
};

enum class GMGameObjectType
{
	Static,
	Entity,
	Sprite,
	Particles,
	Custom,
};

struct GMModelContainerItemIndex;
class GMGameObject : public GMObject
{
	DECLARE_PRIVATE(GMGameObject)

public:
	GMGameObject(AUTORELEASE GMModel* obj);
	GMGameObject(GMGameWorld& world, const GMModelContainerItemIndex& objIndex);
	virtual ~GMGameObject();

public:
	void setModel(AUTORELEASE GMModel* obj);
	GMModel* getModel();

	virtual void setWorld(GMGameWorld* world);
	GMGameWorld* getWorld();
	virtual void onAppendingObjectToWorld() {}
	virtual void onRemovingObjectFromWorld() {}
	virtual GMGameObjectType getType() { return GMGameObjectType::Static; }
	virtual void draw();
	virtual void simulate() {}
	virtual void updateAfterSimulate() {}
	virtual bool canDeferredRendering();

private:
	inline void updateMatrix();

public:
	inline void setScaling(const linear_math::Matrix4x4& scaling) { D(d); updateMatrix(); d->scaling = scaling; }
	inline void setTranslate(const linear_math::Matrix4x4& translation) { D(d); updateMatrix(); d->translation = translation; }
	inline void setRotation(const linear_math::Quaternion& rotation) { D(d); updateMatrix(); d->rotation = rotation; }
	inline const linear_math::Matrix4x4& getTransform() { D(d); return d->transformMatrix; }

protected:
	inline void setDestructor(GMGameObjectDestructor destructor) { D(d); d->destructor = destructor; }

	// events
private:
	static void onShaderSetBlend(GMObject* sender, GMObject* receiver);
};

//GMGlyphObject
struct GlyphProperties
{
};

GM_PRIVATE_OBJECT(GMGlyphObject)
{
	GMString lastRenderText;
	GMString text;
	GlyphProperties properties;
	GMfloat left, bottom, width, height;
	ITexture* texture;
};

class GMComponent;
class GMGlyphObject : public GMGameObject
{
	DECLARE_PRIVATE(GMGlyphObject)

public:
	GMGlyphObject();

public:
	void setText(const GMWchar* text);
	void setGeometry(GMfloat left, GMfloat bottom, GMfloat width, GMfloat height);
	void update();

private:
	virtual void draw() override;
	virtual void onAppendingObjectToWorld() override;
	virtual bool canDeferredRendering() override { return false; }

private:
	void constructModel();
	void updateModel();
	void createVertices(GMComponent* component);
};

//GMEntityObject
enum { EntityPlaneNum = 6 };

GM_PRIVATE_OBJECT(GMEntityObject)
{
	linear_math::Vector3 mins, maxs;
	GMPlane planes[EntityPlaneNum];
};

class GMEntityObject : public GMGameObject
{
	DECLARE_PRIVATE(GMEntityObject)

public:
	GMEntityObject(GMGameWorld& world, const GMModelContainerItemIndex& objIndex);

public:
	virtual GMGameObjectType getType() { return GMGameObjectType::Entity; }

	GMPlane* getPlanes();
	void getBounds(REF linear_math::Vector3& mins, REF linear_math::Vector3& maxs);

private:
	void calc();
	void makePlanes();
};

// GMSkyObject
GM_PRIVATE_OBJECT(GMSkyGameObject)
{
	linear_math::Vector3 min;
	linear_math::Vector3 max;
	Shader shader;
};

class GMSkyGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMSkyGameObject)

public:
	GMSkyGameObject(const Shader& shader, const linear_math::Vector3& min, const linear_math::Vector3& max);

private:
	void createSkyBox(OUT GMModel** obj);
};

END_NS
#endif
