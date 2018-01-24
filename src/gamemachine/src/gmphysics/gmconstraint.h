#ifndef __GMCONSTRAINT_H__
#define __GMCONSTRAINT_H__
#include <gmcommon.h>
#include "gmbulletforward.h"

BEGIN_NS

class GMRigidPhysicsObject;
GM_PRIVATE_OBJECT(GMConstraint)
{
	btTypedConstraint* constraint = nullptr;
	GMRigidPhysicsObject* body = nullptr;
};

class GMConstraint : public GMObject
{
	DECLARE_PRIVATE(GMConstraint)

public:
	GMConstraint(GMRigidPhysicsObject* body);
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
	GMConstraintSetting setting;
};

class GMPoint2PointConstraint : public GMConstraint
{
	DECLARE_PRIVATE_AND_BASE(GMPoint2PointConstraint, GMConstraint)

public:
	GMPoint2PointConstraint(GMRigidPhysicsObject* body, const glm::vec3& pivotA);

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