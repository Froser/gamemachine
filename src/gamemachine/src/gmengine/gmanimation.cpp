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
		d->timeLast = GM.getRunningStates().elapsedTime;
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
	d->keyframesIter = d->keyframes.cbegin();
	d->lastKeyframe = nullptr;
	d->timeline = 0;
	d->isPlaying = false;
}

void GMAnimation::update()
{
	D(d);
	if (d->isPlaying)
	{
		d->timeline += GM.getRunningStates().elapsedTime - d->timeLast;

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
			// ... 考虑是否重播
			return;
		}

		bool newFrameBegun = (currentKeyFrame != d->lastKeyframe);
		if (newFrameBegun)
			d->lastKeyframe = currentKeyFrame;

		for (auto gameObject : d->targetObjects)
		{
			if (newFrameBegun)
				currentKeyFrame->begin(gameObject, d->timeline);
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

void GMGameObjectKeyframe::begin(GMObject* object, GMfloat time)
{
	D(d);
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);

	GMFloat4 translation, scaling;
	GetTranslationFromMatrix(gameObj->getTranslation(), translation);
	GetScalingFromMatrix(gameObj->getScaling(), scaling);

	d->translationMap[gameObj].setFloat4(translation);
	d->scalingMap[gameObj].setFloat4(scaling);
	d->rotationMap[gameObj] = gameObj->getRotation();
	d->timeBegin = time;
}

void GMGameObjectKeyframe::update(GMObject* object, GMfloat time)
{
	D(d);
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	GMfloat percentage = (time - d->timeBegin) / (getTime() - d->timeBegin);

	GMFloat4 translationStart, scalingStart;
	GetTranslationFromMatrix(gameObj->getTranslation(), translationStart);
	GetScalingFromMatrix(gameObj->getScaling(), scalingStart);
	GMVec4 t, s;
	t.loadFloat4(translationStart);
	s.loadFloat4(scalingStart);
	
	gameObj->beginUpdateTransform();
	gameObj->setTranslation(Translate(Lerp(t, d->translationMap[gameObj], percentage)));
	gameObj->setScaling(Scale(Lerp(s, d->scalingMap[gameObj], percentage)));
	gameObj->setRotation(Lerp(gameObj->getRotation(), d->rotationMap[gameObj], percentage));
	gameObj->endUpdateTransform();
}
