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

GMPoint2PointConstraint::GMPoint2PointConstraint(GMRigidPhysicsObject* body, const glm::vec3& pivotA)
	: GMConstraint(body)
{
	D_BASE(d, Base);
	d->body = body;
	d->constraint = new btPoint2PointConstraint(*body->getRigidBody(), btVector3(pivotA[0], pivotA[1], pivotA[2]));
}