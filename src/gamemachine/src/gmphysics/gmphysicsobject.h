#ifndef __PHYSICSOBJECT_H__
#define __PHYSICSOBJECT_H__
#include <gmcommon.h>
#include <map>
#include <linearmath.h>
#include "gmbulletforward.h"
#include <gmassets.h>

BEGIN_NS

GM_ALIGNED_STRUCT(GMPhysicsMoveArgs)
{
	GMPhysicsMoveArgs() = default;
	GMPhysicsMoveArgs(const glm::vec3& _lookAt, const glm::vec3& _direction, const glm::vec3& _speed, const glm::vec3& _rate)
		: lookAt(_lookAt)
		, direction(_direction)
		, speed(_speed)
		, rate(_rate)
	{
	}

	glm::vec3 lookAt; //!< 物体朝向
	glm::vec3 direction; //!< 物体移动方向，坐标系相对于物体朝向，采用右手坐标系，与朝向坐标系一致。
	glm::vec3 speed;
	glm::vec3 rate;
};

class GMRigidPhysicsObject;
GM_ALIGNED_STRUCT(GMPhysicsRayTestResult)
{
	glm::vec3 rayFromWorld = glm::zero<glm::vec3>();
	glm::vec3 rayToWorld = glm::zero<glm::vec3>();
	glm::vec3 hitPointWorld = glm::zero<glm::vec3>();
	glm::vec3 hitNormalWorld = glm::zero<glm::vec3>();
	GMRigidPhysicsObject* hitObject = nullptr;
	bool hit = false;
};

GM_ALIGNED_STRUCT(GMMotionStates)
{
	glm::mat4 transform= glm::identity<glm::mat4>();
	glm::vec3 linearVelocity = glm::zero<glm::vec3>();
};

GM_PRIVATE_OBJECT(GMPhysicsObject)
{
	GMGameObject* gameObject = nullptr;
	GMMotionStates motionStates;
};

//! 表示一个物理对象
/*!
  物理对象拥有一些物理性质，如边界大小等。你不能直接创建它，而是应该根据GMPhysicsWorld::newPhysicsObject()来创建相应的GMPhysicsObject。
  \sa GMPhysicsWorld
*/
class GMPhysicsObject : public GMObject
{
	DECLARE_PRIVATE(GMPhysicsObject)
	friend class GMPhysicsWorld;
	friend class GMGameObject;

protected:
	GMPhysicsObject() = default;

public:
	//! 获取物理对象的运动状态。
	/*!
	  物理对象运动状态包括全局变换、线速度等。
	  \return 物理对象的运动状态。
	*/
	virtual const GMMotionStates& getMotionStates();

public:
	inline void setMotionStates(const GMMotionStates& motionStates)
	{
		D(d);
		d->motionStates = motionStates;
	}

private:
	//! 将此物理对象与一个GMGameObject绑定起来
	/*!
	  在调用GMGameObject::setPhysicsObject时，此方法将会被GMGameObject调用，因此设置为私有。此对象生命周期由所绑定的GMGameObject管理。
	  \sa GMGameObject::setPhysicsObject()
	*/
	inline void setGameObject(GMGameObject* gameObject)
	{
		D(d);
		GM_ASSERT(!d->gameObject);
		d->gameObject = gameObject;
	}

public:
	//! 获取所绑定的GMGameObject对象。
	/*!
	  本对象的生命周期由所绑定的GMGameObject对象所管理。
	  \return 本对象所绑定的GMGameObject对象。
	*/
	GMGameObject* getGameObject()
	{
		D(d);
		return d->gameObject;
	}
};

class GMPhysicsShape;
GM_PRIVATE_OBJECT(GMRigidPhysicsObject)
{
	GMint updateRevision = -1;
	btRigidBody* body = nullptr; // btRigidBody在添加到物理世界后，应该由物理世界管理声明周期
	bool bodyDetached = false;
	btDefaultMotionState* motionState = nullptr;
	GMPhysicsShape* shape = nullptr;
	GMfloat mass = 0;
};

class GMRigidPhysicsObject : public GMPhysicsObject
{
	DECLARE_PRIVATE_AND_BASE(GMRigidPhysicsObject, GMPhysicsObject)

	friend class GMDiscreteDynamicsWorld;

public:
	GMRigidPhysicsObject() = default;
	~GMRigidPhysicsObject();

public:
	//! 将一个物理形状与此物理对象相关联。
	/*!
	  可以用同一个物理形状关联多个物理对象。此方法将构造出物理对象的所有属性，因此，应该在其调用之前设置好所有的物理参数，并且与一个GMGameObject绑定。
	  \param shape GMAssetType为PhysicsShape的物理形状资产。
	  \sa GMAsset, GMGameObject::setPhysicsObject()
	*/
	void setShape(GMAsset shape);

	void setMass(GMfloat mass) { D(d); d->mass = mass; }
	bool isStaticObject() const;
	bool isKinematicObject() const;
	bool isStaticOrKinematicObject() const;
	bool hasContactResponse() const;

public:
	//! 获取物理对象的运动状态。
	/*!
	  物理对象运动状态包括全局变换、线速度等。在本类的实现中，它先会检查运动状态是否更新，如果有更新，则将更新后的值缓存到类的内部，如果没有更新，则返回原值。
	  \return 物理对象的运动状态。
	*/
	virtual const GMMotionStates& getMotionStates() override;

private:
	void initRigidBody(GMfloat mass, const btTransform& startTransform, const glm::vec3& color);

	//! 解除对一个bullet刚体的生命周期管理。
	/*!
	  在一个bullet刚体被创建时，它的生命周期由此类的实例管理。此类实例析构时，bullet刚体被释放。<br>
	  但是，如果这个bullet刚体被添加到了物理世界中，则调用此方法解除对此刚体生命周期的管理，此bullet刚体生命周期由物理世界管理。
	  \sa GMDiscreteDynamicsWorld::addRigidObject()
	*/
	void detachRigidBody();

//GMDiscreteDynamicsWorld
	btRigidBody* getRigidBody()
	{
		D(d);
		GM_ASSERT(d->body);
		return d->body;
	}

	GMPhysicsShape* getShape()
	{
		D(d);
		GM_ASSERT(d->shape);
		return d->shape;
	}
};

END_NS
#endif
