#ifndef __GMANIMATION_H__
#define __GMANIMATION_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMAnimationKeyframe)
{
	GMfloat time;
};

//! 动画的关键帧。
class GMAnimationKeyframe : public GMObject
{
	GM_DECLARE_PRIVATE(GMAnimationKeyframe)
	GM_DECLARE_PROPERTY(Time, time, GMfloat);

public:
	//! 关键帧处理方法，用于改变GMObject对象的状态。
	/*!
	  当当前关键帧为目标关键帧时，每当需要更新GMObject状态时，此方法被调用。
	  \param object 希望执行动画的对象。
	  \param time 当前动画执行的时间。
	*/
	virtual void update(GMObject* object, GMfloat time) = 0;

public:
	bool operator <(const GMAnimationKeyframe& rhs)
	{
		return getTime() < rhs.getTime();
	}
};

GM_PRIVATE_OBJECT(GMAnimation)
{
	bool playLoop = false;
	bool isPlaying = false;
	GMfloat timeline = 0;
	GMfloat timeLast = 0;
	Set<GMOwnedPtr<GMAnimationKeyframe>> keyframes;
	Set<GMOwnedPtr<GMAnimationKeyframe>>::const_iterator keyframesIter;
	Set<GMObject*> targetObjects;
};

class GMAnimation : public GMObject
{
	GM_DECLARE_PRIVATE(GMAnimation)
	GM_DECLARE_PROPERTY(PlayLoop, playLoop, bool)

public:
	template <typename... GameObject>
	GMAnimation(GameObject*... targetGameObjects)
	{
		D(d);
		d->targetObjects = { targetGameObjects };
	}

	template <>
	GMAnimation(void) = default;

public:
	template <typename... GameObject>
	void setTargetObjects(GameObject*... targetGameObjects)
	{
		D(d);
		d->targetObjects = { targetGameObjects };
	}

	inline bool isPlaying() GM_NOEXCEPT
	{
		D(d);
		return d->isPlaying;
	}

public:
	void addKeyFrame(AUTORELEASE GMAnimationKeyframe* kf);
	void play();
	void pause();
	void reset();
	void update();

private:
	void updatePercentage();
};

END_NS
#endif