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

void HallucinationGameObject::appendThisObjectToWorld(btDynamicsWorld* world)
{
}

void HallucinationGameObject::initPhysics(btDynamicsWorld* world)
{
	// 不起作用
}

btCollisionObject* HallucinationGameObject::createCollisionObject()
{
	return nullptr;
}

btCollisionShape* HallucinationGameObject::createCollisionShape()
{
	return nullptr;
}

void HallucinationGameObject::getReadyForRender(DrawingList& list)
{
	D(d);
	btTransform trans = d.transform;
	GMfloat glTrans[16];
	trans.getOpenGLMatrix(glTrans);

	vmath::mat4 M = getTransformMatrix(glTrans);
	vmath::mat4 S = vmath::scale(m_scale[0], m_scale[1], m_scale[2]);

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

const btVector3& HallucinationGameObject::getLocalScaling()
{
	return m_scale;
}