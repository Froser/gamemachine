#include "stdafx.h"
#include "gmconstraint.h"
#include "gmbulletincludes.h"
#include "gmphysicsobject.h"
#include <linearmath.h>

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMConstraint)
{
	btTypedConstraint* constraint = nullptr;
};

GMConstraint::GMConstraint()
{
	GM_CREATE_DATA();
}

GMConstraint::~GMConstraint()
{
	D(d);
	GM_delete(d->constraint);
}

btTypedConstraint* GMConstraint::getConstraint()
{
	D(d);
	return d->constraint;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMPoint2PointConstraint)
{
	btPoint2PointConstraint* constraint = nullptr;
	GMConstraintSetting setting;
	GMRigidPhysicsObject* bodyA = nullptr;
	GMRigidPhysicsObject* bodyB = nullptr;
};

GMPoint2PointConstraint::GMPoint2PointConstraint(GMRigidPhysicsObject* body, const GMVec3& pivotA)
{
	GM_CREATE_DATA();

	D(d);
	D_BASE(db, Base);
	d->bodyA = body;
	d->constraint = new btPoint2PointConstraint(*body->getRigidBody(), btVector3(pivotA.getX(), pivotA.getY(), pivotA.getZ()));
	db->constraint = d->constraint;
}

GMPoint2PointConstraint::GMPoint2PointConstraint(GMRigidPhysicsObject* bodyA, GMRigidPhysicsObject* bodyB, const GMVec3& pivotA, const GMVec3& pivotB)
{
	GM_CREATE_DATA();

	D(d);
	D_BASE(db, Base);
	d->bodyA = bodyA;
	d->bodyB = bodyB;
	d->constraint = new btPoint2PointConstraint(
		*bodyA->getRigidBody(), 
		*bodyB->getRigidBody(),
		btVector3(pivotA.getX(), pivotA.getY(), pivotA.getZ()),
		btVector3(pivotB.getX(), pivotB.getY(), pivotB.getZ())
	);
	db->constraint = d->constraint;
}

void GMPoint2PointConstraint::setPivotA(const GMVec3& pivot)
{
	D(d);
	d->constraint->setPivotA(btVector3(pivot.getX(), pivot.getY(), pivot.getZ()));
}

void GMPoint2PointConstraint::setPivotB(const GMVec3& pivot)
{
	D(d);
	d->constraint->setPivotB(btVector3(pivot.getX(), pivot.getY(), pivot.getZ()));
}

void GMPoint2PointConstraint::setConstraintSetting(const GMConstraintSetting& setting)
{
	D(d);
	d->setting = setting;
}

const GMConstraintSetting& GMPoint2PointConstraint::getConstraintSetting()
{
	D(d);
	return d->setting;
}

END_NS
