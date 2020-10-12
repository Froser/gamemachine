#ifndef __GMANIMATION_H__
#define __GMANIMATION_H__
#include <gmcommon.h>
BEGIN_NS

struct IInterpolationFloat { virtual GMfloat interpolate(const GMfloat&, const GMfloat&, GMfloat) = 0; };
struct IInterpolationVec3 { virtual GMVec3 interpolate(const GMVec3&, const GMVec3&, GMfloat) = 0; };
struct IInterpolationVec4 { virtual GMVec4 interpolate(const GMVec4&, const GMVec4&, GMfloat) = 0; };
struct IInterpolationQuat { virtual GMQuat interpolate(const GMQuat&, const GMQuat&, GMfloat) = 0; };

template <typename T>
struct ProperInterpolationInterface_;

template <>
struct ProperInterpolationInterface_<GMfloat>
{
	typedef IInterpolationFloat Type;
};

template <>
struct ProperInterpolationInterface_<GMVec3>
{
	typedef IInterpolationVec3 Type;
};

template <>
struct ProperInterpolationInterface_<GMVec4>
{
	typedef IInterpolationVec4 Type;
};

template <>
struct ProperInterpolationInterface_<GMQuat>
{
	typedef IInterpolationQuat Type;
};

template <typename T>
using ProperInterpolationInterface = typename ProperInterpolationInterface_<T>::Type;

struct GM_EXPORT GMInterpolationFunctors
{
	GMSharedPtr<IInterpolationFloat> floatFunctor;
	GMSharedPtr<IInterpolationVec3> vec3Functor;
	GMSharedPtr<IInterpolationVec4> vec4Functor;
	GMSharedPtr<IInterpolationQuat> quatFunctor;

	static GMInterpolationFunctors getDefaultInterpolationFunctors();
	static void binarySearch(const AlignedVector<GMVec2>& points, GMfloat x, GMVec2& resultA, GMVec2& resultB);
};

template <typename T>
struct GMLerpFunctor : ProperInterpolationInterface<T>
{
	T interpolate(const T& p0, const T& p1, GMfloat percentage)
	{
		return Lerp(p0, p1, percentage);
	}
};

template <typename T>
struct GMCubicBezierFunctor : ProperInterpolationInterface<T>
{
	GMCubicBezierFunctor(const GMVec2& cp0, const GMVec2& cp1, GMint32 slice = 60)
	{
		GM_ASSERT(0 <= cp0.getX() && cp0.getX() <= 1);
		GM_ASSERT(0 <= cp1.getX() && cp1.getX() <= 1);

		// 构造一个从0~1的贝塞尔曲线，根据percentage来获取变换的进度
		auto tFunc = [cp0, cp1](GMfloat t)
		{
			return (
				cp0 * 3 * t * Pow(1 - t, 2) +
				cp1 * 3 * t * t * (1 - t) +
				GMVec2(1, 1) * Pow(t, 3));
		};

		GMfloat dt = 1.f / slice;
		m_interpolations.resize(slice + 1);
		for (GMfloat i = 0; i < slice; ++i)
		{
			GMfloat t = dt * i;
			m_interpolations[i] = tFunc(t);
		}
		m_interpolations[slice] = GMVec2(1, 1);
	}

	T interpolate(const T& p0, const T& p1, GMfloat percentage)
	{
		// 根据二分法查找percentage所在的曲线的最近的两个点，并进行插值
		GMVec2 a, b;
		GMfloat t = 0;
		GMInterpolationFunctors::binarySearch(m_interpolations, percentage, a, b);
		GM_ASSERT(b.getX() >= a.getX());
		if (a == b)
		{
			// 如果给定的点恰好就是容器中的采样点
			t = a.getY();
		}
		else
		{
			// 两点之间进行插值
			GMfloat d = (percentage - a.getX()) / (b.getX() - a.getX());
			t = Lerp(a, b, d).getY();
		}
		return Lerp(p0, p1, t);
	}

private:
	AlignedVector<GMVec2> m_interpolations;
};

//! 动画的关键帧。
GM_PRIVATE_CLASS(GMAnimationKeyframe);
class GM_EXPORT GMAnimationKeyframe : public GMObject
{
	GM_DECLARE_PRIVATE(GMAnimationKeyframe)
	GM_DECLARE_PROPERTY(GMfloat, Time);
	GM_DECLARE_PROPERTY(GMInterpolationFunctors, Functors);

public:
	GMAnimationKeyframe(GMfloat timePoint);
	~GMAnimationKeyframe();

	virtual void reset(IDestroyObject* object) = 0;
	virtual void beginFrame(IDestroyObject* object, GMfloat timeStart) = 0;
	virtual void endFrame(IDestroyObject* object) = 0;

	//! 关键帧处理方法，用于改变GMObject对象的状态。
	/*!
	  当当前关键帧为目标关键帧时，每当需要更新GMObject状态时，此方法被调用。
	  \param object 希望执行动画的对象。
	  \param time 当前动画执行的时间。从前一帧结束开始算起。
	*/
	virtual void update(IDestroyObject* object, GMfloat time) = 0;

public:
	bool operator <(const GMAnimationKeyframe& rhs) const
	{
		return getTime() < rhs.getTime();
	}
};

class GMAnimationKeyframeLess
{
public:
	bool operator ()(const GMAnimationKeyframe* lhs, const GMAnimationKeyframe* rhs) const
	{
		return *lhs < *rhs;
	}
};

GM_PRIVATE_OBJECT_UNALIGNED(GMAnimation)
{
	bool playLoop = false;
	bool isPlaying = false;
	bool finished = false;
	GMDuration timeline = 0;
	Set<GMAnimationKeyframe*, GMAnimationKeyframeLess> keyframes;
	Set<GMAnimationKeyframe*, GMAnimationKeyframeLess>::const_iterator keyframesIter;
	Set<IDestroyObject*> targetObjects;
	GMAnimationKeyframe* lastKeyframe = nullptr;
};

class GM_EXPORT GMAnimation : public GMObject
{
	GM_DECLARE_PRIVATE(GMAnimation)
	GM_DECLARE_EMBEDDED_PROPERTY(PlayLoop, playLoop)

public:
	template <typename... Objects>
	GMAnimation(Objects... targetGameObjects)
		: GMAnimation()
	{
		setTargetObjects(targetGameObjects...);
	}

#if GM_MSVC
	template <>
	GMAnimation() { GM_CREATE_DATA(); }
#elif GM_GCC
	GMAnimation() { GM_CREATE_DATA(); }
#endif

	GMAnimation(const GMAnimation& rhs)
	{
		GM_COPY(rhs);
	}

	GMAnimation(GMAnimation&& rhs) GM_NOEXCEPT
	{
		GM_MOVE(rhs);
	}

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

	inline bool isFinished() GM_NOEXCEPT
	{
		D(d);
		return d->finished;
	}

public:
	void clearObjects();
	void clearFrames();
	void addKeyFrame(AUTORELEASE GMAnimationKeyframe* kf);
	void play();
	void pause();
	void reset();
	void update(GMDuration dt);

private:
	void updatePercentage();
};

// Animations
struct GMGameObjectKeyframeComponent
{
	enum
	{
		NoComponent = 0x00,
		Translate = 0x01,
		Scale = 0x02,
		Rotate = 0x04,
	};
};

GM_PRIVATE_CLASS(GMGameObjectKeyframe);
class GM_EXPORT GMGameObjectKeyframe : public GMAnimationKeyframe
{
	GM_DECLARE_PRIVATE(GMGameObjectKeyframe)
	GM_DECLARE_PROPERTY(GMVec4, Translation)
	GM_DECLARE_PROPERTY(GMVec4, Scaling)
	GM_DECLARE_PROPERTY(GMQuat, Rotation)

public:
	GMGameObjectKeyframe(
		GMint32 component,
		const GMVec4& translation,
		const GMVec4& scaling,
		const GMQuat& rotation,
		GMfloat timePoint
	);

	~GMGameObjectKeyframe();

public:
	virtual void reset(IDestroyObject* object) override;
	virtual void beginFrame(IDestroyObject* object, GMfloat timeStart) override;
	virtual void endFrame(IDestroyObject* object) override;
	virtual void update(IDestroyObject* object, GMfloat time) override;
};

enum class GMCameraKeyframeComponent
{
	NoComponent,
	FocusAt,
	LookAtDirection,
};

GM_PRIVATE_CLASS(GMCameraKeyframe);
class GM_EXPORT GMCameraKeyframe : public GMAnimationKeyframe
{
	GM_DECLARE_PRIVATE(GMCameraKeyframe)
	GM_DECLARE_BASE(GMAnimationKeyframe)
	GM_DECLARE_PROPERTY(GMVec3, Position)
	GM_DECLARE_PROPERTY(GMVec3, LookAtDirection)
	GM_DECLARE_PROPERTY(GMVec3, FocusAt)

public:
	GMCameraKeyframe(
		GMCameraKeyframeComponent component,
		const GMVec3& position,
		const GMVec3& lookAtDirectionOrFocusAt,
		GMfloat timePoint
	);

	~GMCameraKeyframe();

public:
	virtual void reset(IDestroyObject* object) override;
	virtual void beginFrame(IDestroyObject* object, GMfloat timeStart) override;
	virtual void endFrame(IDestroyObject* object) override;
	virtual void update(IDestroyObject* object, GMfloat time) override;
};

struct GMLightKeyframeComponent
{
	enum
	{
		NoComponent = 0x00,
		Ambient = 0x01,
		Diffuse = 0x02,
		Specular = 0x04,
		CutOff = 0x08,
		Position = 0x10,
	};
};

GM_PRIVATE_CLASS(GMLightKeyframe);
class GM_EXPORT GMLightKeyframe : public GMAnimationKeyframe
{
	GM_DECLARE_PRIVATE(GMLightKeyframe)
	GM_DECLARE_BASE(GMAnimationKeyframe)
	GM_DECLARE_PROPERTY(GMVec3, Ambient)
	GM_DECLARE_PROPERTY(GMVec3, Diffuse)
	GM_DECLARE_PROPERTY(GMfloat, Specular)
	GM_DECLARE_PROPERTY(GMfloat, CutOff)
	GM_DECLARE_PROPERTY(GMVec3, Position)

public:
	GMLightKeyframe(
		const IRenderContext* context,
		GMint32 component,
		const GMVec3& ambient,
		const GMVec3& diffuse,
		GMfloat specular,
		const GMVec3& position,
		GMfloat cutOff,
		GMfloat timePoint
	);

	~GMLightKeyframe();

public:
	virtual void reset(IDestroyObject* object) override;
	virtual void beginFrame(IDestroyObject* object, GMfloat timeStart) override;
	virtual void endFrame(IDestroyObject* object) override;
	virtual void update(IDestroyObject* object, GMfloat time) override;
};

END_NS
#endif