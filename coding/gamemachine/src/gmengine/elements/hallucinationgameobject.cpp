#include "stdafx.h"
#include "hallucinationgameobject.h"

HallucinationGameObject::HallucinationGameObject(AUTORELEASE Object* obj)
{
	setObject(obj);
}

void HallucinationGameObject::setLocalScaling(const btVector3& scale)
{
	m_scale = scale;
}

void HallucinationGameObject::appendObjectToWorld(btDynamicsWorld* world)
{
}

void HallucinationGameObject::setMass(btScalar)
{
	// 不起作用
}

btCollisionShape* HallucinationGameObject::createCollisionShape()
{
	return nullptr;
}

void HallucinationGameObject::getReadyForRender(DrawingList& list)
{
	btTransform trans = dataRef().getTransform();
	GMfloat glTrans[16];
	trans.getOpenGLMatrix(glTrans);

	vmath::mat4 M = getScalingAndTransformMatrix(glTrans, m_scale);
	if (dataRef().animationState() == Running)
		M = M * getAnimationMatrix();

	DrawingItem item;
	memcpy(item.trans, M, sizeof(M));
	item.gameObject = this;
	list.push_back(item);
}