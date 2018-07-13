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
	GMAnimationKeyframe() = default;

	virtual void reset(GMObject* object) = 0;
	virtual void beginFrame(GMObject* object, GMfloat timeStart) = 0;
	virtual void endFrame(GMObject* object) = 0;

	//! 关键帧处理方法，用于改变GMObject对象的状态。
	/*!
	  当当前关键帧为目标关键帧时，每当需要更新GMObject状态时，此方法被调用。
	  \param object 希望执行动画的对象。
	  \param time 当前动画执行的时间。从前一帧结束开始算起。
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
	Set<GMAnimationKeyframe*> keyframes;
	Set<GMAnimationKeyframe*>::const_iterator keyframesIter;
	Set<GMObject*> targetObjects;
	GMAnimationKeyframe* lastKeyframe = nullptr;
};

class GMAnimation : public GMObject
{
	GM_DECLARE_PRIVATE(GMAnimation)
	GM_ALLOW_COPY_DATA(GMAnimation)
	GM_DECLARE_PROPERTY(PlayLoop, playLoop, bool)

public:
	template <typename... Objects>
	GMAnimation(Objects... targetGameObjects)
	{
		setTargetObjects(targetGameObjects...);
	}

	template <>
	GMAnimation(void) = default;

	~GMAnimation();

public:
	template <typename... Object>
	void setTargetObjects(Object*... targetGameObjects)
	{
		D(d);
		d->targetObjects = { targetGameObjects... };
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

// Animations
GM_PRIVATE_OBJECT(GMGameObjectKeyframe)
{
	Map<GMGameObject*, GMVec4> translationMap;
	Map<GMGameObject*, GMVec4> scalingMap;
	Map<GMGameObject*, GMQuat> rotationMap;
	GMVec4 translation;
	GMVec4 scaling;
	GMQuat rotation;
	GMfloat timeStart = 0;
};

class GMGameObjectKeyframe : public GMAnimationKeyframe
{
	GM_DECLARE_PRIVATE(GMGameObjectKeyframe)
	GM_DECLARE_PROPERTY(Translation, translation, GMVec4)
	GM_DECLARE_PROPERTY(Scaling, scaling, GMVec4)
	GM_DECLARE_PROPERTY(Rotation, rotation, GMQuat)

public:
	GMGameObjectKeyframe(
		const GMVec4& translation,
		const GMVec4& scaling,
		const GMQuat& rotation,
		GMfloat timePoint
	);

public:
	virtual void reset(GMObject* object) override;
	virtual void beginFrame(GMObject* object, GMfloat timeStart) override;
	virtual void endFrame(GMObject* object) override;
	virtual void update(GMObject* object, GMfloat time) override;
};
END_NS
#endif