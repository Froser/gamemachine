#ifndef __GMCONSTRAINT_H__
#define __GMCONSTRAINT_H__
#include <gmcommon.h>
#include "gmbulletforward.h"

struct GMVec3;

BEGIN_NS

class GMRigidPhysicsObject;
GM_PRIVATE_OBJECT(GMConstraint)
{
	btTypedConstraint* constraint = nullptr;
};

class GMConstraint : public GMObject
{
	GM_DECLARE_PRIVATE(GMConstraint)

public:
	GMConstraint();
	~GMConstraint();

public:
	btTypedConstraint* getConstraint()
	{
		D(d);
		return d->constraint;
	}

protected:
	template <typename T>
	void upcast()
	{
		D(d);
		return static_cast<T>(d->constraint);
	}
};

struct GMConstraintSetting
{
	GMfloat tau = .3f;;
	GMfloat damping = 1.f;
	GMfloat impulseClamp = 0.f;
};

GM_PRIVATE_OBJECT(GMPoint2PointConstraint)
{
	btPoint2PointConstraint* constraint = nullptr;
	GMConstraintSetting setting;
	GMRigidPhysicsObject* bodyA = nullptr;
	GMRigidPhysicsObject* bodyB = nullptr;
};

class GMPoint2PointConstraint : public GMConstraint
{
	GM_DECLARE_PRIVATE_AND_BASE(GMPoint2PointConstraint, GMConstraint)

public:
	GMPoint2PointConstraint(GMRigidPhysicsObject* body, const GMVec3& pivotA);
	GMPoint2PointConstraint(GMRigidPhysicsObject* bodyA, GMRigidPhysicsObject* bodyB, const GMVec3& pivotA, const GMVec3& pivotB);

public:
	void setPivotA(const GMVec3& pivot);
	void setPivotB(const GMVec3& pivot);

public:
	const GMConstraintSetting& getConstraintSetting()
	{
		D(d);
		return d->setting;
	}

	void setConstraintSetting(const GMConstraintSetting& setting)
	{
		D(d);
		d->setting = setting;
	}
};

END_NS
#endif