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

GM_ALIGNED_STRUCT(GMMotionStates)
{
	glm::vec3 translation = glm::zero<glm::vec3>();
	glm::vec3 velocity = glm::zero<glm::vec3>();
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
	inline GMMotionStates& motionStates() { D(d); return d->motionStates; }

private:
	inline void setGameObject(GMGameObject* gameObject)
	{
		D(d);
		GM_ASSERT(!d->gameObject);
		d->gameObject = gameObject;
	}

public:
	GMGameObject* getGameObject()
	{
		D(d);
		return d->gameObject;
	}
};

class GMPhysicsShape;
GM_PRIVATE_OBJECT(GMRigidPhysicsObject)
{
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
	void setMass(GMfloat mass);

private:
	void initRigidBody(GMfloat mass, const btTransform& startTransform, const glm::vec3& color);
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
