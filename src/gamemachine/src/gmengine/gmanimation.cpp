#include "stdafx.h"
#include "gmanimation.h"
#include "foundation/gamemachine.h"

void GMAnimation::addKeyFrame(AUTORELEASE GMAnimationKeyframe* kf)
{
	D(d);
	d->keyframes.insert(GMOwnedPtr<GMAnimationKeyframe>(kf));
	d->keyframesIter = d->keyframes.cbegin();
}

void GMAnimation::play()
{
	D(d);
	if (!d->isPlaying)
	{
		d->timeline = 0;
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
				currentKeyFrame = keyframe.get();
				break;
			}
		}

		if (d->keyframesIter == d->keyframes.cend())
		{
			// 最后一帧关键帧也播放完了
			// ... 考虑是否重播
		}

		for (auto gameObject : d->targetObjects)
		{
			currentKeyFrame->update(gameObject, d->timeline);
		}
	}
}