#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <gmmodel.h>
#include <linearmath.h>
#include <gmphysicsworld.h>

BEGIN_NS

GM_PRIVATE_OBJECT(GMGameObject)
{
	GMuint32 id = 0;
	GMOwnedPtr<GMPhysicsObject> physics;
	GMGameWorld* world = nullptr;
	const IRenderContext* context = nullptr;
	bool autoUpdateTransformMatrix = true;
	GMAsset asset;

	GM_ALIGNED_16(struct)
	{
		GMMat4 scaling = Identity<GMMat4>();
		GMMat4 translation = Identity<GMMat4>();
		GMQuat rotation = Identity<GMQuat>();
		GMMat4 transformMatrix = Identity<GMMat4>();
	} transforms;

	struct
	{
		ITechnique* currentTechnique = nullptr;
	} drawContext;
};

class GM_EXPORT GMGameObject : public GMObject
{
	GM_DECLARE_PRIVATE(GMGameObject)

public:
	GMGameObject() = default;
	GMGameObject(GMSceneAsset asset);
	~GMGameObject() = default;

public:
	void setAsset(GMSceneAsset asset);
	GMScene* getScene();
	GMModel* getModel();
	void setWorld(GMGameWorld* world);
	GMGameWorld* getWorld();
	void setPhysicsObject(AUTORELEASE GMPhysicsObject* phyObj);
	void foreach(std::function<void(GMModel*)>);

public:
	virtual void onAppendingObjectToWorld() {}
	virtual void onRemovingObjectFromWorld() {}
	virtual void draw();
	virtual void update(GMDuration dt) {}
	virtual bool canDeferredRendering();
	virtual const IRenderContext* getContext();
	virtual bool isAnimationObject() const;

protected:
	virtual void drawModel(const IRenderContext* context, GMModel* model);
	virtual void endDraw();

public:
	void updateTransformMatrix();
	void setScaling(const GMMat4& scaling);
	void setTranslation(const GMMat4& translation);
	void setRotation(const GMQuat& rotation);
	void beginUpdateTransform();
	void endUpdateTransform();

	inline const GMMat4& getTransform() const GM_NOEXCEPT
	{
		D(d);
		return d->transforms.transformMatrix;
	}

	inline const GMMat4& getScaling() const GM_NOEXCEPT {
		D(d);
		return d->transforms.scaling;
	}

	inline const GMMat4& getTranslation() const GM_NOEXCEPT
	{
		D(d);
		return d->transforms.translation;
	}

	inline const GMQuat& getRotation() const GM_NOEXCEPT {
		D(d);
		return d->transforms.rotation;
	}

	inline GMPhysicsObject* getPhysicsObject()
	{
		D(d);
		return d->physics.get();
	}

	inline void setContext(const IRenderContext* context)
	{
		D(d);
		d->context = context;
	}

private:
	inline void setAutoUpdateTransformMatrix(bool autoUpdateTransformMatrix) GM_NOEXCEPT
	{
		D(d);
		d->autoUpdateTransformMatrix = autoUpdateTransformMatrix;
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

class GM_EXPORT GMCubeMapGameObject : public GMGameObject
{
public:
	GMCubeMapGameObject(GMTextureAsset texture);

public:
	//! 将立方体贴图从渲染中移除
	/*!
	  很多渲染效果都需要用到立方体贴图。因此，当不再使用天空盒时，一定要手动调用这个方法，清除渲染中的天空盒。否则基于立方体贴图的绘制将会产生问题。
	  在立方体贴图对象被析构的时候，也会先调用此方法清除渲染环境中的立方体贴图。
	*/
	void deactivate();

private:
	void createCubeMap(GMTextureAsset texture);

public:
	virtual bool canDeferredRendering() override;
};

END_NS
#endif
