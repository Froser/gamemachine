#include "stdafx.h"
#include "gmanimation.h"
#include "foundation/gamemachine.h"
#include "gameobjects/gmgameobject.h"

GMAnimation::~GMAnimation()
{
	D(d);
	for (auto kf : d->keyframes)
	{
		GM_delete(kf);
	}
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
			// 考虑是否重播
			if (d->playLoop)
			{
				reset();
				play();
			}
			else
			{
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

		for (auto gameObject : d->targetObjects)
		{
			currentKeyFrame->update(gameObject, d->timeline);
		}
	}
}

GMGameObjectKeyframe::GMGameObjectKeyframe(
	const GMVec4& translation,
	const GMVec4& scaling,
	const GMQuat& rotation,
	GMfloat timePoint
)
{
	D(d);
	setTranslation(translation);
	setScaling(scaling);
	setRotation(rotation);
	setTime(timePoint);
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
