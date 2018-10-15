#include "stdafx.h"
#include "gmconstraint.h"
#include "gmbulletincludes.h"
#include "gmphysicsobject.h"
#include <linearmath.h>

GMConstraint::GMConstraint()
{
}

GMConstraint::~GMConstraint()
{
	D(d);
	GM_delete(d->constraint);
}

GMPoint2PointConstraint::GMPoint2PointConstraint(GMRigidPhysicsObject* body, const GMVec3& pivotA)
{
	D(d);
	D_BASE(db, Base);
	d->bodyA = body;
	d->constraint = new btPoint2PointConstraint(*body->getRigidBody(), btVector3(pivotA.getX(), pivotA.getY(), pivotA.getZ()));
	db->constraint = d->constraint;
}

GMPoint2PointConstraint::GMPoint2PointConstraint(GMRigidPhysicsObject* bodyA, GMRigidPhysicsObject* bodyB, const GMVec3& pivotA, const GMVec3& pivotB)
{
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