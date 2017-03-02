#include "stdafx.h"
#include "gameobject.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmengine/elements/gameworld.h"

GameObject::GameObject()
{

}

void GameObject::setId(GMuint id)
{
	D(d);
	d.id = id;
}

GMuint GameObject::getId()
{
	D(d);
	return d.id;
}

void GameObject::setObject(AUTORELEASE Object* obj)
{
	D(d);
	d.object.reset(obj);
}

Object* GameObject::getObject()
{
	D(d);
	if (!d.object)
		d.object.reset(new Object());
	return d.object;
}

void GameObject::setWorld(GameWorld* world)
{
	D(d);
	ASSERT(!d.world);
	d.world = world;
}

GameWorld* GameObject::getWorld()
{
	D(d);
	return d.world;
}

void GameObject::getReadyForRender(DrawingList& list)
{
	D(d);
	vmath::mat4 M = vmath::mat4::identity();

	if (d.animationState == Running)
	{
		AnimationMatrices mat = getAnimationMatrix();
		M = mat.tranlation * M * mat.rotation * mat.scaling;
	}

	DrawingItem item;
	memcpy(item.trans, M, sizeof(M));
	item.gameObject = this;
	list.push_back(item);
}

void GameObject::onAppendingObjectToWorld()
{

}

vmath::mat4 GameObject::getTransformMatrix(GMfloat glTrans[16])
{
	vmath::mat4 T(
		vmath::vec4(glTrans[0], glTrans[1], glTrans[2], glTrans[3]),
		vmath::vec4(glTrans[4], glTrans[5], glTrans[6], glTrans[7]),
		vmath::vec4(glTrans[8], glTrans[9], glTrans[10], glTrans[11]),
		vmath::vec4(glTrans[12], glTrans[13], glTrans[14], glTrans[15])
	);
	return T;
}

AnimationMatrices GameObject::getAnimationMatrix()
{
	D(d);
	GMfloat current = d.world->getElapsed();
	GMfloat start = d.animationStartTick;
	GMfloat percentage = current / (start + d.animationDuration);
	if (percentage > 1)
		percentage -= (int)percentage;

	vmath::quaternion rotation = d.keyframesRotation.isEmpty() ? vmath::quaternion(1, 0, 0, 0) : d.keyframesRotation.calculateInterpolation(percentage, true);
	vmath::quaternion translation = d.keyframesTranslation.isEmpty() ? vmath::quaternion(0, 0, 0, 0) : d.keyframesTranslation.calculateInterpolation(percentage, false);
	vmath::quaternion scaling = d.keyframesScaling.isEmpty() ? vmath::quaternion(1, 1, 1, 1) : d.keyframesScaling.calculateInterpolation(percentage, false);

	AnimationMatrices mat = {
		vmath::rotate(rotation[3], rotation[0], rotation[1], rotation[2]),
		vmath::translate(translation[0], translation[1], translation[2]),
		vmath::scale(scaling[0], scaling[1], scaling[2]),
	};

	return mat;
}

Keyframes& GameObject::getKeyframesRotation()
{
	D(d);
	return d.keyframesRotation;
}

Keyframes& GameObject::getKeyframesTranslation()
{
	D(d);
	return d.keyframesTranslation;
}

Keyframes& GameObject::getKeyframesScaling()
{
	D(d);
	return d.keyframesScaling;
}

void GameObject::startAnimation(GMuint duration)
{
	D(d);
	d.animationStartTick = d.world ? d.world->getElapsed() : 0;
	d.animationDuration = duration;
	d.animationState = Running;
}

void GameObject::stopAnimation()
{
	D(d);
	d.animationState = Stopped;
}