#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include <gmcommon.h>
#include <tools.h>
#include <gmmodel.h>
#include <linearmath.h>
#include <gmphysicsworld.h>

BEGIN_NS

typedef Vector<GMModel*> GMModels;

GM_PRIVATE_OBJECT(GMGameObject)
{
	GMuint id = 0;
	GMPhysicsObject* physics = nullptr;
	GMGameWorld* world = nullptr;
	GMModels models;
	GMMat4 scaling = Identity<GMMat4>();
	GMMat4 translation = Identity<GMMat4>();
	GMQuat rotation = Identity<GMQuat>();
	GMMat4 transformMatrix = Identity<GMMat4>();
	bool canDeferredRendering = true;
	bool forceDisableDeferredRendering = false;
	GMString name;
};

enum class GMGameObjectType
{
	Static,
	Entity, // 可响应实体
	Sprite, // 精灵
	Particles, // 粒子
	Custom,
};

struct GMAsset;
class GMGameObject : public GMObject
{
	DECLARE_PRIVATE(GMGameObject)

public:
	GMGameObject() = default;
	GMGameObject(GMAsset asset);
	~GMGameObject();

public:
	void addModel(GMAsset asset);
	GMModels& getModels();

	virtual void setWorld(GMGameWorld* world);
	GMGameWorld* getWorld();
	virtual void onAppendingObjectToWorld() {}
	virtual void onRemovingObjectFromWorld() {}
	virtual GMGameObjectType getType() { return GMGameObjectType::Static; }
	virtual void draw();
	virtual void updateAfterSimulate() {}
	virtual void simulate() {}
	virtual bool canDeferredRendering();

private:
	inline void updateMatrix()
	{ 
		D(d);
		d->transformMatrix = d->scaling * QuatToMatrix(d->rotation) * d->translation;
	}

public:
	virtual void setScaling(const GMMat4& scaling) { D(d); d->scaling = scaling; updateMatrix(); }
	virtual void setTranslation(const GMMat4& translation) { D(d); d->translation = translation; updateMatrix(); }
	virtual void setRotation(const GMQuat& rotation) { D(d); d->rotation = rotation; updateMatrix(); }
	inline const GMMat4& getTransform() const { D(d); return d->transformMatrix; }
	inline const GMMat4& getScaling() const { D(d); return d->scaling; }
	inline const GMMat4& getTranslation() const { D(d); return d->translation; }
	inline const GMQuat& getRotation() const { D(d); return d->rotation; }
	inline GMPhysicsObject* getPhysicsObject() { D(d); GM_ASSERT(d->physics); return d->physics; }
	inline void setPhysicsObject(AUTORELEASE GMPhysicsObject* phyObj)
	{
		D(d);
		d->physics = phyObj;
		d->physics->setGameObject(this);
	}

	inline void forceDisableDeferredRendering()
	{
		D(d);
		d->forceDisableDeferredRendering = true;
	}
};

// GMSkyObject
// 一个天空的盒子，用6个面模拟一个天空
GM_PRIVATE_OBJECT(GMBSPSkyGameObject)
{
	GMVec3 min;
	GMVec3 max;
	GMShader shader;
};

class GMCubeMapGameObject : public GMGameObject
{
public:
	GMCubeMapGameObject(ITexture* texture);
	~GMCubeMapGameObject();

public:
	//! 将立方体贴图从渲染中移除
	/*!
	  很多渲染效果都需要用到立方体贴图。因此，当不再使用天空盒时，一定要手动调用这个方法，清除渲染中的天空盒。否则基于立方体贴图的绘制将会产生问题。
	  在立方体贴图对象被析构的时候，也会先调用此方法清除渲染环境中的立方体贴图。
	*/
	void deactivate();

private:
	void createCubeMap(ITexture* texture);

public:
	virtual bool canDeferredRendering() override;
};

END_NS
#endif
