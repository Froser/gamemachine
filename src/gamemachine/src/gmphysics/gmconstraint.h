#ifndef __GMCONSTRAINT_H__
#define __GMCONSTRAINT_H__
#include <gmcommon.h>
#include "gmbulletforward.h"

struct GMVec3;

BEGIN_NS

class GMRigidPhysicsObject;
class GM_EXPORT GMConstraint : public GMObject
{
	GM_DECLARE_PRIVATE(GMConstraint)

public:
	GMConstraint();
	~GMConstraint();

public:
	btTypedConstraint* getConstraint();

protected:
	template <typename T>
	void upcast()
	{
		D(d);
		return static_cast<T>(getConstraint());
	}
};

struct GMConstraintSetting
{
	GMfloat tau = .3f;;
	GMfloat damping = 1.f;
	GMfloat impulseClamp = 0.f;
};

GM_PRIVATE_CLASS(GMPoint2PointConstraint);
class GM_EXPORT GMPoint2PointConstraint : public GMConstraint
{
	GM_DECLARE_PRIVATE(GMPoint2PointConstraint)
	GM_DECLARE_BASE(GMConstraint)

public:
	GMPoint2PointConstraint(GMRigidPhysicsObject* body, const GMVec3& pivotA);
	GMPoint2PointConstraint(GMRigidPhysicsObject* bodyA, GMRigidPhysicsObject* bodyB, const GMVec3& pivotA, const GMVec3& pivotB);

public:
	void setPivotA(const GMVec3& pivot);
	void setPivotB(const GMVec3& pivot);

public:
	const GMConstraintSetting& getConstraintSetting();
	void setConstraintSetting(const GMConstraintSetting& setting);
};

END_NS
#endif