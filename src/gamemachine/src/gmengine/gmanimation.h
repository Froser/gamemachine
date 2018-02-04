#ifndef __GMANIMATION_H__
#define __GMANIMATION_H__
#include <gmcommon.h>
BEGIN_NS

typedef std::function<GMVec3(const GMVec3&, const GMVec3&, GMfloat)> GMInterpolation;
typedef std::function<GMQuat(const GMQuat&, const GMQuat&, GMfloat)> GMQuaternionInterpolation;

struct GMInterpolations
{
	static GMVec3 linear(const GMVec3& start, const GMVec3& end, GMfloat p)
	{
		return Lerp(start, end, p);
	}

	static GMQuat quaternion_linear(const GMQuat& start, const GMQuat& end, GMfloat p)
	{
		return Lerp(start, end, p);
	}
};

struct GMAnimationTypes
{
	enum Types
	{
		BeginType,
		Scaling = BeginType,
		Translation,
		Rotation,
		EndType,
	};
};

enum class GMAnimationPlayingState
{
	Activated,
	Deactivated,
};

GM_ALIGNED_STRUCT(GMAnimationState)
{
	GMfloat tick = 0;
	GMfloat p = 0;
	GMVec3 start;
	GMVec3 end;
	GMQuat start_q; // only used in rotation
	GMQuat end_q; // only used in rotation
	GMQuaternionInterpolation interpolation_q; // only used in rotation
	GMint direction = 1;
	GMInterpolation interpolation;
	GMAnimationPlayingState playingState = GMAnimationPlayingState::Deactivated;
	bool set = false;
};

GM_PRIVATE_OBJECT(GMAnimation)
{
	GMAnimationState animationStates[GMAnimationTypes::EndType];
	GMGameObject* object = nullptr;
	GMfloat duration = 0;
	bool canStart = true;
	bool canReverse = false;
	bool canResume = false;
};

class GMAnimation : public GMObject
{
	DECLARE_PRIVATE(GMAnimation)

public:
	GMAnimation(GMGameObject* object);

public:
	inline virtual bool canStart() { D(d); return d->canStart; }
	virtual void start();
	inline virtual bool canReverse() { D(d); return d->canReverse; }
	virtual void reverse();
	inline virtual bool canResume() { D(d); return d->canResume; }
	virtual void resume();
	virtual void update();

public:
	void setDuration(GMfloat duration);
	void setScaling(const GMVec3& scaling, GMInterpolation interpolation = GMInterpolations::linear);
	void disableScaling();
	void setTranslation(const GMVec3& translation, GMInterpolation interpolation = GMInterpolations::linear);
	void disableTranslation();
	void setRotation(const GMQuat& rotation, GMQuaternionInterpolation interpolation = GMInterpolations::quaternion_linear);
	void disableRotation();

private:
	void startAnimation(GMAnimationTypes::Types type);
};

class GMControlGameObject;
GM_PRIVATE_OBJECT(GMControlGameObjectAnimation)
{
	GMControlGameObject* object = nullptr;
};

class GMControlGameObjectAnimation : public GMAnimation
{
	DECLARE_PRIVATE_AND_BASE(GMControlGameObjectAnimation, GMAnimation)

public:
	GMControlGameObjectAnimation(GMControlGameObject* object);

public:
	void setTranslation(GMint x, GMint y, GMInterpolation interpolation = GMInterpolations::linear);
};

END_NS
#endif