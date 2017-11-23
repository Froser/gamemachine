#ifndef __GMANIMATION_H__
#define __GMANIMATION_H__
#include <gmcommon.h>
BEGIN_NS

typedef std::function<linear_math::Vector3(const linear_math::Vector3&, const linear_math::Vector3&, GMfloat)> GMInterpolation;
typedef std::function<linear_math::Quaternion(const linear_math::Quaternion&, const linear_math::Quaternion&, GMfloat)> GMQuaternionInterpolation;

struct GMInterpolations
{
	static linear_math::Vector3 linear(const linear_math::Vector3& start, const linear_math::Vector3& end, GMfloat p)
	{
		return linear_math::lerp(start, end, p);
	}

	static linear_math::Quaternion quaternion_linear(const linear_math::Quaternion& start, const linear_math::Quaternion& end, GMfloat p)
	{
		return linear_math::slerp(start, end, p);
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
	linear_math::Vector3 start;
	linear_math::Vector3 end;
	linear_math::Quaternion start_q; // only used in rotation
	linear_math::Quaternion end_q; // only used in rotation
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
	void setScaling(const linear_math::Vector3& scaling, GMInterpolation interpolation = GMInterpolations::linear);
	void disableScaling();
	void setTranslation(const linear_math::Vector3& translation, GMInterpolation interpolation = GMInterpolations::linear);
	void disableTranslation();
	void setRotation(const linear_math::Quaternion& rotation, GMQuaternionInterpolation interpolation = GMInterpolations::quaternion_linear);
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
	DECLARE_PRIVATE(GMControlGameObjectAnimation)

public:
	GMControlGameObjectAnimation(GMControlGameObject* object);

public:
	void setTranslation(GMint x, GMint y, GMInterpolation interpolation = GMInterpolations::linear);
};

END_NS
#endif