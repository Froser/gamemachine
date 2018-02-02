#include "stdafx.h"
#include "gmconstraint.h"
#include "gmbulletincludes.h"
#include "gmphysicsobject.h"
#include <linearmath.h>

GMConstraint::GMConstraint(GMRigidPhysicsObject* body)
{
	D(d);
	d->body = body;
}

GMConstraint::~GMConstraint()
{
	D(d);
	GM_delete(d->constraint);
}

GMPoint2PointConstraint::GMPoint2PointConstraint(GMRigidPhysicsObject* body, const GMVec3& pivotA)
	: GMConstraint(body)
{
	D(d);
	D_BASE(db, Base);
	db->body = body;
	d->constraint = new btPoint2PointConstraint(*body->getRigidBody(), btVector3(pivotA[0], pivotA[1], pivotA[2]));
	db->constraint = d->constraint;
}

void GMPoint2PointConstraint::setPivotA(const GMVec3& pivot)
{
	D(d);
	d->constraint->setPivotA(btVector3(pivot[0], pivot[1], pivot[2]));
}

void GMPoint2PointConstraint::setPivotB(const GMVec3& pivot)
{
	D(d);
	d->constraint->setPivotB(btVector3(pivot[0], pivot[1], pivot[2]));
}