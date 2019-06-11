#ifndef __GMANIMATION_H__
#define __GMANIMATION_H__
#include <gmcommon.h>
BEGIN_NS

struct IInterpolationFloat { virtual GMfloat interpolate(GMfloat, GMfloat, GMfloat) = 0; };
struct IInterpolationVec3 { virtual GMVec3 interpolate(GMVec3, GMVec3, GMfloat) = 0; };
struct IInterpolationVec4 { virtual GMVec4 interpolate(GMVec4, GMVec4, GMfloat) = 0; };
struct IInterpolationQuat { virtual GMQuat interpolate(GMQuat, GMQuat, GMfloat) = 0; };

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

struct GMInterpolationFunctors
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
	T interpolate(T p0, T p1, GMfloat percentage)
	{
		return Lerp(p0, p1, percentage);
	}
};

template <typename T>
struct GMCubicBezierFunctor : ProperInterpolationInterface<T>
{
	GMCubicBezierFunctor(GMVec2 cp0, GMVec2 cp1, GMint32 slice = 20)
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

	T interpolate(T p0, T p1, GMfloat percentage)
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

GM_PRIVATE_OBJECT(GMAnimationKeyframe)
{
	GMfloat time;
	GMInterpolationFunctors functors;
};

//! 动画的关键帧。
class GM_EXPORT GMAnimationKeyframe : public GMObject
{
	GM_DECLARE_PRIVATE(GMAnimationKeyframe)
	GM_DECLARE_PROPERTY(Time, time);
	GM_DECLARE_PROPERTY(Functors, functors);

public:
	GMAnimationKeyframe(GMfloat timePoint);

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
	bool operator ()(const GMAnimationKeyframe* lhs, const GMAnimationKeyframe* rhs)
	{
		return *lhs < *rhs;
	}
};

GM_PRIVATE_OBJECT(GMAnimation)
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
	GM_ALLOW_COPY_MOVE(GMAnimation)
	GM_DECLARE_PROPERTY(PlayLoop, playLoop)

public:
	template <typename... Objects>
	GMAnimation(Objects... targetGameObjects)
	{
		setTargetObjects(targetGameObjects...);
	}

#if GM_MSVC
	template <>
	GMAnimation(void) {};
#elif GM_GCC
	GMAnimation() = default;
#endif

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
GM_PRIVATE_OBJECT(GMGameObjectKeyframe)
{
	Map<GMGameObject*, GMVec4, std::less<GMGameObject*>, AlignedAllocator<Pair<GMGameObject*, GMVec4>> > translationMap;
	Map<GMGameObject*, GMVec4, std::less<GMGameObject*>, AlignedAllocator<Pair<GMGameObject*, GMVec4>> > scalingMap;
	Map<GMGameObject*, GMQuat, std::less<GMGameObject*>, AlignedAllocator<Pair<GMGameObject*, GMQuat>> > rotationMap;
	GMVec4 translation;
	GMVec4 scaling;
	GMQuat rotation;
	GMfloat timeStart = 0;
};

class GM_EXPORT GMGameObjectKeyframe : public GMAnimationKeyframe
{
	GM_DECLARE_PRIVATE(GMGameObjectKeyframe)
	GM_DECLARE_PROPERTY(Translation, translation)
	GM_DECLARE_PROPERTY(Scaling, scaling)
	GM_DECLARE_PROPERTY(Rotation, rotation)

public:
	GMGameObjectKeyframe(
		const GMVec4& translation,
		const GMVec4& scaling,
		const GMQuat& rotation,
		GMfloat timePoint
	);

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

GM_PRIVATE_OBJECT(GMCameraKeyframe)
{
	Map<GMCamera*, GMVec3, std::less<GMCamera*>, AlignedAllocator<Pair<GMGameObject*, GMVec3>> > positionMap;
	Map<GMCamera*, GMVec3, std::less<GMCamera*>, AlignedAllocator<Pair<GMGameObject*, GMVec3>> > lookAtDirectionMap;
	Map<GMCamera*, GMVec3, std::less<GMCamera*>, AlignedAllocator<Pair<GMGameObject*, GMVec3>> > focusMap;
	GMCameraKeyframeComponent component = GMCameraKeyframeComponent::NoComponent;
	GMVec3 position;
	GMVec3 lookAtDirection;
	GMVec3 focusAt;
	GMfloat timeStart = 0;
};

class GM_EXPORT GMCameraKeyframe : public GMAnimationKeyframe
{
	GM_DECLARE_PRIVATE_AND_BASE(GMCameraKeyframe, GMAnimationKeyframe)
	GM_DECLARE_PROPERTY(Position, position)
	GM_DECLARE_PROPERTY(LookAtDirection, lookAtDirection)
	GM_DECLARE_PROPERTY(FocusAt, focusAt)

public:
	GMCameraKeyframe(
		GMCameraKeyframeComponent component,
		const GMVec3& position,
		const GMVec3& lookAtDirectionOrFocusAt,
		GMfloat timePoint
	);

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
	};
};

GM_PRIVATE_OBJECT(GMLightKeyframe)
{
	Map<ILight*, GMVec3, std::less<ILight*>, AlignedAllocator<Pair<GMGameObject*, GMVec3>> > ambientMap;
	Map<ILight*, GMVec3, std::less<ILight*>, AlignedAllocator<Pair<GMGameObject*, GMVec3>> > diffuseMap;
	Map<ILight*, GMfloat, std::less<ILight*>> specularMap;
	GMint32 component = GMLightKeyframeComponent::NoComponent;
	GMVec3 ambient;
	GMVec3 diffuse;
	GMfloat specular;
	GMfloat timeStart = 0;
	const IRenderContext* context = nullptr;
};

class GM_EXPORT GMLightKeyframe : public GMAnimationKeyframe
{
	GM_DECLARE_PRIVATE_AND_BASE(GMLightKeyframe, GMAnimationKeyframe)
	GM_DECLARE_PROPERTY(Ambient, ambient)
	GM_DECLARE_PROPERTY(Diffuse, diffuse)
	GM_DECLARE_PROPERTY(Specular, specular)

public:
	GMLightKeyframe(
		const IRenderContext* context,
		GMint32 component,
		const GMVec3& ambient,
		const GMVec3& diffuse,
		GMfloat specular,
		GMfloat timePoint
	);

public:
	virtual void reset(IDestroyObject* object) override;
	virtual void beginFrame(IDestroyObject* object, GMfloat timeStart) override;
	virtual void endFrame(IDestroyObject* object) override;
	virtual void update(IDestroyObject* object, GMfloat time) override;
};

END_NS
#endif