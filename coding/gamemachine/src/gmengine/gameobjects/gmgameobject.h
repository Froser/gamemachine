#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include <gmcommon.h>
#include <utilities.h>
#include <gmmodel.h>
#include <linearmath.h>

BEGIN_NS

GM_PRIVATE_OBJECT(GMGameObject)
{
	GMuint id = 0;
	GMGameWorld* world = nullptr;
	GMModel* model = nullptr;
	glm::mat4 scaling = glm::identity<glm::mat4>();
	glm::mat4 translation = glm::identity<glm::mat4>();
	glm::quat rotation = glm::identity<glm::quat>();
	glm::mat4 transformMatrix = glm::identity<glm::mat4>();

	bool canDeferredRendering = true;
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

public:
	void setModel(GMAsset asset);
	GMModel* getModel();

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
		d->transformMatrix = d->translation * glm::mat4_cast(d->rotation) * d->scaling; 
	}

public:
	inline virtual void setScaling(const glm::mat4& scaling) { D(d); d->scaling = scaling; updateMatrix(); }
	inline virtual void setTranslation(const glm::mat4& translation) { D(d); d->translation = translation; updateMatrix(); }
	inline virtual void setRotation(const glm::quat& rotation) { D(d); d->rotation = rotation; updateMatrix(); }
	inline const glm::mat4& getTransform() const { D(d); return d->transformMatrix; }
	inline const glm::mat4& getScaling() const { D(d); return d->scaling; }
	inline const glm::mat4& getTranslation() const { D(d); return d->translation; }
	inline const glm::quat& getRotation() const { D(d); return d->rotation; }

	// events
private:
	static void onShaderSetBlend(GMObject* sender, GMObject* receiver);
};

//GMEntityObject
enum { EntityPlaneNum = 6 };

GM_PRIVATE_OBJECT(GMEntityObject)
{
	glm::vec3 mins, maxs;
	GMPlane planes[EntityPlaneNum];
};

class GMEntityObject : public GMGameObject
{
	DECLARE_PRIVATE_AND_BASE(GMEntityObject, GMGameObject)

public:
	GMEntityObject(GMAsset asset);

public:
	virtual GMGameObjectType getType() { return GMGameObjectType::Entity; }

	GMPlane* getPlanes();
	void getBounds(REF glm::vec3& mins, REF glm::vec3& maxs);

private:
	void calc();
	void makePlanes();
};

// GMSkyObject
// 一个天空的盒子，用6个面模拟一个天空
GM_PRIVATE_OBJECT(GMSkyGameObject)
{
	glm::vec3 min;
	glm::vec3 max;
	GMShader shader;
};

class GMSkyGameObject : public GMGameObject
{
	DECLARE_PRIVATE_AND_BASE(GMSkyGameObject, GMGameObject)

public:
	GMSkyGameObject(const GMShader& shader, const glm::vec3& min, const glm::vec3& max);
	~GMSkyGameObject();

private:
	void createSkyBox(OUT GMModel** obj);
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
