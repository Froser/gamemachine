#include "stdafx.h"
#include "convexhullgameobject.h"
#include "gmdatacore/object.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

ConvexHullGameObject::ConvexHullGameObject(AUTORELEASE Object* obj)
{
	setObject(obj);
}

btCollisionShape* ConvexHullGameObject::createCollisionShape()
{
	return new btConvexHullShape(getObject()->vertices().data(), getObject()->vertices().size() / 4, sizeof(Object::DataType) * 4 );
}

void ConvexHullGameObject::getReadyForRender(DrawingList& list)
{
	D(d);
	btTransform trans = d.transform;
	GMfloat glTrans[16];
	trans.getOpenGLMatrix(glTrans);

	vmath::mat4 M = getTransformMatrix(glTrans);
	const btVector3 scaling = d.collisionShape->getLocalScaling();
	vmath::mat4 S = vmath::scale(scaling[0], scaling[1], scaling[2]);

	if (d.animationState == Running)
	{
		AnimationMatrices mat = getAnimationMatrix();
		M = mat.tranlation * M * mat.rotation * mat.scaling;
	}

	DrawingItem item;
	memcpy(item.trans, M * S, sizeof(M));
	item.gameObject = this;
	list.push_back(item);
}