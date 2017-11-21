#ifndef __GMANIMATION_H__
#define __GMANIMATION_H__
#include <gmcommon.h>
BEGIN_NS

typedef std::function<linear_math::Vector3(const linear_math::Vector3&, const linear_math::Vector3&, float)> GMInterpolation;
struct GMInterpolations
{
	static linear_math::Vector3 linear(const linear_math::Vector3& start, const linear_math::Vector3& end, GMfloat p)
	{
		return linear_math::lerp(start, end, p);
	}
};

struct GMAnimationTypes
{
	enum Types
	{
		BeginType,
		Scaling = BeginType,
		Translation,
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
	GMfloat duration = 0;
	linear_math::Vector3 start;
	linear_math::Vector3 end;
	GMint direction = 1;
	GMInterpolation interpolation;
	GMAnimationPlayingState playingState = GMAnimationPlayingState::Deactivated;
	bool canStart = true;
	bool canReverse = false;
	bool canResume = false;
	bool set = false;
};

GM_PRIVATE_OBJECT(GMAnimation)
{
	GMAnimationState animationStates[GMAnimationTypes::EndType];
	GMGameObject* object = nullptr;
};

class GMAnimation : public GMObject
{
	DECLARE_PRIVATE(GMAnimation)

public:
	GMAnimation(GMGameObject* object);

public:
	virtual bool canStart();
	virtual void start();
	virtual bool canReverse();
	virtual void reverse();
	virtual bool canResume();
	virtual void resume();
	virtual void update();

public:
	void setScaling(const linear_math::Vector3& scaling, GMfloat duration, GMInterpolation interpolation = GMInterpolations::linear);
	void disableScaling();
	void setTranslation(const linear_math::Vector3& translation, GMfloat duration, GMInterpolation interpolation = GMInterpolations::linear);
	void disableTranslation();

private:
	void startAnimation();
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
	void setTranslation(GMint x, GMint y, GMfloat duration, GMInterpolation interpolation = GMInterpolations::linear);
};

END_NS
#endif