#include "stdafx.h"
#include "gmanimation.h"
#include "foundation/gamemachine.h"
#include "gameobjects/gmgameobject.h"
#include <gmcamera.h>

namespace
{
	GMVec3 SlerpVector(
		GMVec3 const& x,
		GMVec3 const& y,
		GMfloat percentage
	)
	{
		// get cosine of angle between vectors (-1 -> 1)
		GMfloat CosAlpha = Dot(x, y);
		// get angle (0 -> pi)
		GMfloat Alpha = Acos(CosAlpha);
		// get sine of angle between vectors (0 -> 1)
		GMfloat SinAlpha = Sin(Alpha);
		// this breaks down when SinAlpha = 0, i.e. Alpha = 0 or pi
		GMfloat t1 = Sin((static_cast<GMfloat>(1) - percentage) * Alpha) / SinAlpha;
		GMfloat t2 = Sin(percentage * Alpha) / SinAlpha;

		// interpolate src vectors
		return x * t1 + y * t2;
	}
}

GMAnimation::~GMAnimation()
{
	D(d);
	for (auto kf : d->keyframes)
	{
		GM_delete(kf);
	}
}

void GMAnimation::clearObjects()
{
	D(d);
	pause();
	d->targetObjects.clear();
}

void GMAnimation::clearFrames()
{
	D(d);
	pause();
	d->keyframes.clear();
}

void GMAnimation::addKeyFrame(AUTORELEASE GMAnimationKeyframe* kf)
{
	D(d);
	d->keyframes.insert(kf);
	d->keyframesIter = d->keyframes.cbegin();
}

void GMAnimation::play()
{
	D(d);
	if (!d->isPlaying)
	{
		d->isPlaying = true;
		d->finished = false;
	}
}

void GMAnimation::pause()
{
	D(d);
	if (d->isPlaying)
		d->isPlaying = false;
}

void GMAnimation::reset()
{
	D(d);
	if (!d->keyframes.empty())
		d->keyframesIter = d->keyframes.cbegin();
	d->lastKeyframe = nullptr;
	d->timeline = 0;
	d->isPlaying = false;
	for (auto object : d->targetObjects)
	{
		if (d->keyframesIter != d->keyframes.cend())
			(*d->keyframesIter)->reset(object);
	}
}

void GMAnimation::update(GMDuration dt)
{
	D(d);
	if (d->isPlaying)
	{
		d->timeline += dt;

		// 目标关键帧，缓存当前迭代器，增加查找效率
		GMAnimationKeyframe* currentKeyFrame = nullptr;
		if (d->keyframes.empty())
			return;

		for (; d->keyframesIter != d->keyframes.cend(); ++d->keyframesIter)
		{
			decltype(auto) keyframe = *d->keyframesIter;
			if (d->timeline <= keyframe->getTime())
			{
				currentKeyFrame = keyframe;
				break;
			}
		}

		if (d->keyframesIter == d->keyframes.cend())
		{
			// 最后一帧关键帧也播放完了
			d->finished = true;
			// 考虑是否重播
			if (d->playLoop)
			{
				reset();
				play();
			}
			else
			{
				// 更新最后一帧防止错过
				if (!d->keyframes.empty())
				{
					for (auto object : d->targetObjects)
					{
						auto last = *d->keyframes.rbegin();
						last->update(object, d->timeline);
					}
				}
				pause();
			}
			return;
		}

		if (currentKeyFrame != d->lastKeyframe)
		{
			// 结束老的一帧，开始新的一帧
			for (auto object : d->targetObjects)
			{
				if (d->lastKeyframe)
					d->lastKeyframe->endFrame(object);
				currentKeyFrame->beginFrame(object, d->lastKeyframe ? d->lastKeyframe->getTime() : 0);
			}
			d->lastKeyframe = currentKeyFrame;
		}

		for (auto object : d->targetObjects)
		{
			currentKeyFrame->update(object, d->timeline);
		}
	}
}

GMAnimationKeyframe::GMAnimationKeyframe(GMfloat timePoint)
{
	setTime(timePoint);
}

GMGameObjectKeyframe::GMGameObjectKeyframe(
	const GMVec4& translation,
	const GMVec4& scaling,
	const GMQuat& rotation,
	GMfloat timePoint
)
	: GMAnimationKeyframe(timePoint)
{
	D(d);
	setTranslation(translation);
	setScaling(scaling);
	setRotation(rotation);
}

void GMGameObjectKeyframe::reset(GMObject* object)
{
	D(d);
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	gameObj->setTranslation(Translate(d->translationMap[gameObj]));
	gameObj->setScaling(Scale(d->scalingMap[gameObj]));
	gameObj->setRotation(d->rotationMap[gameObj]);
}

void GMGameObjectKeyframe::beginFrame(GMObject* object, GMfloat timeStart)
{
	D(d);
	d->timeStart = timeStart;
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);

	GMFloat4 translation, scaling;
	GetTranslationFromMatrix(gameObj->getTranslation(), translation);
	GetScalingFromMatrix(gameObj->getScaling(), scaling);

	d->translationMap[gameObj].setFloat4(translation);
	d->scalingMap[gameObj].setFloat4(scaling);
	d->rotationMap[gameObj] = gameObj->getRotation();
}

void GMGameObjectKeyframe::endFrame(GMObject* object)
{
	D(d);
	// 去掉误差，把对象放到正确的位置
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	gameObj->beginUpdateTransform();
	gameObj->setTranslation(Translate(getTranslation()));
	gameObj->setScaling(Scale(getScaling()));
	gameObj->setRotation(getRotation());
	gameObj->endUpdateTransform();
}

void GMGameObjectKeyframe::update(GMObject* object, GMfloat time)
{
	D(d);
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	GMfloat percentage = (time - d->timeStart) / (getTime() - d->timeStart);
	gameObj->beginUpdateTransform();
	gameObj->setTranslation(Translate(Lerp(d->translationMap[gameObj], getTranslation(), percentage)));
	gameObj->setScaling(Scale(Lerp(d->scalingMap[gameObj], getScaling(), percentage)));
	gameObj->setRotation(Lerp(d->rotationMap[gameObj], getRotation(), percentage));
	gameObj->endUpdateTransform();
}

//////////////////////////////////////////////////////////////////////////
GMCameraKeyframe::GMCameraKeyframe(const GMVec3& position, const GMVec3& lookAtDirection, GMfloat timePoint)
	: GMAnimationKeyframe(timePoint)
{
	D(d);
	setPosition(position);
	setLookAtDirection(lookAtDirection);
}

void GMCameraKeyframe::reset(GMObject* object)
{
	D(d);
	GMCamera* camera = gm_cast<GMCamera*>(object);
	GMCameraLookAt lookAt(d->lookAtDirectionMap[camera], d->positionMap[camera]);
	camera->lookAt(lookAt);
}

void GMCameraKeyframe::beginFrame(GMObject* object, GMfloat timeStart)
{
	D(d);
	d->timeStart = timeStart;
	GMCamera* camera = gm_cast<GMCamera*>(object);
	const GMCameraLookAt& lookAt = camera->getLookAt();
	d->positionMap[camera] = lookAt.position;
	d->lookAtDirectionMap[camera] = lookAt.lookDirection;
}

void GMCameraKeyframe::endFrame(GMObject* object)
{
	D(d);
	// 去掉误差，把对象放到正确的位置
	GMCamera* camera = gm_cast<GMCamera*>(object);
	GMCameraLookAt lookAt;
	lookAt.position = getPosition();
	lookAt.lookDirection = getLookAtDirection();
	camera->lookAt(lookAt);
}

void GMCameraKeyframe::update(GMObject* object, GMfloat time)
{
	D(d);
	GMCamera* camera = gm_cast<GMCamera*>(object);
	GMfloat percentage = (time - d->timeStart) / (getTime() - d->timeStart);
	GMCameraLookAt lookAt = camera->getLookAt();
	lookAt.position = Lerp(d->positionMap[camera], getPosition(), percentage);
	lookAt.lookDirection = Normalize(SlerpVector(d->lookAtDirectionMap[camera], getLookAtDirection(), percentage));

	camera->lookAt(lookAt);
}