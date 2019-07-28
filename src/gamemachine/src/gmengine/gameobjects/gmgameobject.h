#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <gmmodel.h>
#include <linearmath.h>

BEGIN_NS

class GMAnimationGameObjectHelper;
class GMPhysicsObject;

enum class GMGameObjectCullOption
{
	NoCull,
	AABB,
};

enum class GMGameObjectRenderPriority
{
	High,
	Normal,
};

GM_PRIVATE_CLASS(GMGameObject);
class GM_EXPORT GMGameObject : public GMObject
{
	GM_DECLARE_PRIVATE(GMGameObject)
	GM_DECLARE_PROPERTY(GMGameObjectRenderPriority, RenderPriority)

public:
	enum
	{
		InvalidIndex = -1,
	};

public:
	GMGameObject();
	GMGameObject(GMSceneAsset asset);
	~GMGameObject();

public:
	void setAsset(GMSceneAsset asset);
	GMAsset getAsset();
	GMScene* getScene();
	const GMScene* getScene() const;
	void setWorld(GMGameWorld* world);
	GMGameWorld* getWorld();
	void setPhysicsObject(AUTORELEASE GMPhysicsObject* phyObj);
	void foreachModel(std::function<void(GMModel*)>);
	void setCullComputeShaderProgram(IComputeShaderProgram* shaderProgram);

public:
	virtual GMModel* getModel();
	virtual void onAppendingObjectToWorld();
	virtual void onRemovingObjectFromWorld() {}
	virtual void draw();
	virtual void update(GMDuration dt);
	virtual bool canDeferredRendering();
	virtual const IRenderContext* getContext();
	virtual void onRenderShader(GMModel*, IShaderProgram* shaderProgram) const {}

	// 动画控制
public:
	virtual void play();
	virtual void reset(bool update);
	virtual void pause();
	virtual bool isPlaying();

public:
	GMsize_t getAnimationCount();
	void setAnimation(GMsize_t index);
	Vector<GMString> getAnimationNames();
	GMsize_t getAnimationIndexByName(const GMString& name);

protected:
	virtual void drawModel(const IRenderContext* context, GMModel* model);
	virtual void endDraw();
	virtual void makeAABB();
	virtual IComputeShaderProgram* getCullShaderProgram();
	virtual void cull();

public:
	void updateTransformMatrix();
	void setScaling(const GMMat4& scaling);
	void setTranslation(const GMMat4& translation);
	void setRotation(const GMQuat& rotation);
	void beginUpdateTransform();
	void endUpdateTransform();
	void setCullOption(GMGameObjectCullOption option, GMCamera* camera = nullptr);
	GMAnimationType getAnimationType() const;

	bool getVisible() const GM_NOEXCEPT;
	const GMMat4& getTransform() const GM_NOEXCEPT;
	const GMMat4& getScaling() const GM_NOEXCEPT;
	const GMMat4& getTranslation() const GM_NOEXCEPT;
	const GMQuat& getRotation() const GM_NOEXCEPT;
	GMPhysicsObject* getPhysicsObject();
	void setContext(const IRenderContext* context);
	void setVisible(bool visible) const GM_NOEXCEPT;

private:
	void setAutoUpdateTransformMatrix(bool autoUpdateTransformMatrix) GM_NOEXCEPT;
	void releaseAllBufferHandle();

public:
	//! 设置默认的裁剪程序。如果没有设置，那么GMGameObject将会采用CPU裁剪。
	/*!
	  为当前渲染环境（如DirectX11, OpenGL)设置裁剪的计算着色器代码。<BR>
	  着色器的入口一定要为main。<BR>
	*/
	static void setDefaultCullShaderCode(const GMString& code);

private:
	static GMString s_defaultComputeShaderCode;
};

// GMSkyObject
// 一个天空的盒子，用6个面模拟一个天空
GM_PRIVATE_CLASS(GMCubeMapGameObject);
class GM_EXPORT GMCubeMapGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE(GMCubeMapGameObject)

public:
	GMCubeMapGameObject(GMTextureAsset texture);
	~GMCubeMapGameObject();

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
