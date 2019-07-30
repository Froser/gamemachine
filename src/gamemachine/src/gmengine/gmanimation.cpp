#include "stdafx.h"
#include "gmanimation.h"
#include "foundation/gamemachine.h"
#include "gameobjects/gmgameobject.h"
#include <gmcamera.h>
#include <gmlight.h>

BEGIN_NS

#define RUN_AND_CHECK(expr) { bool b = expr; GM_ASSERT(b); }

namespace
{
	GMVec3 SlerpVector(
		GMVec3 const& x,
		GMVec3 const& y,
		GMfloat percentage
	)
	{
		if (x == y)
			return x;

		// get cosine of angle between vectors (-1 -> 1)
		GMfloat CosAlpha = Dot(x, y);
		// get angle (0 -> pi)
		GMfloat Alpha = Acos(CosAlpha);
		// get sine of angle between vectors (0 -> 1)
		GMfloat SinAlpha = Sin(Alpha);
		// this breaks down when SinAlpha = 0, i.e. Alpha = 0 or pi
		GMfloat t1 = Sin((static_cast<GMfloat>(1) - percentage) * Alpha) / SinAlpha;
		GMfloat t2 = Sin(percentage * Alpha) / SinAlpha;

		// interpolate src vectors
		return x * t1 + y * t2;
	}
}

GMInterpolationFunctors GMInterpolationFunctors::getDefaultInterpolationFunctors()
{
	static GMInterpolationFunctors s_default = {
		GMSharedPtr<IInterpolationFloat>(new GMLerpFunctor<GMfloat>()),
		GMSharedPtr<IInterpolationVec3>(new GMLerpFunctor<GMVec3>()),
		GMSharedPtr<IInterpolationVec4>(new GMLerpFunctor<GMVec4>()),
		GMSharedPtr<IInterpolationQuat>(new GMLerpFunctor<GMQuat>()),
	};
	return s_default;
}

void GMInterpolationFunctors::binarySearch(const AlignedVector<GMVec2>& points, GMfloat x, GMVec2& resultA, GMVec2& resultB)
{
	// 从一个有序的顶点序列中找到某个x值相邻的2个点
	GMint32 boundL = 0, boundR = gm_sizet_to_int(points.size() - 1);
	while (boundL <= boundR)
	{
		GMint32 mid = (boundR + boundL) / 2;

		const GMVec2& p = points[mid];
		if (FuzzyCompare(p.getX(), x))
		{
			// 刚好找到这个点
			resultA = resultB = p;
			return;
		}
		else if (p.getX() < x)
		{
			boundL = mid + 1;
		}
		else if (p.getX() > x)
		{
			boundR = mid - 1;
		}
	}
	
	GM_ASSERT(boundR == boundL - 1);
	if (boundR < 0)
		boundR = 0;
	resultA = points[boundR];
	resultB = points[boundL];
}

GMAnimation::~GMAnimation()
{
	if (_gm_data)
	{
		D(d);
		for (auto kf : d->keyframes)
		{
			GM_delete(kf);
		}
	}
}

void GMAnimation::clearObjects()
{
	D(d);
	pause();
	d->targetObjects.clear();
}

void GMAnimation::clearFrames()
{
	D(d);
	pause();
	d->keyframes.clear();
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
		d->finished = false;
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
		if (d->keyframes.empty())
			return;

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
			d->finished = true;
			// 考虑是否重播
			if (d->playLoop)
			{
				reset();
				play();
			}
			else
			{
				// 更新最后一帧防止错过
				if (!d->keyframes.empty())
				{
					for (auto object : d->targetObjects)
					{
						auto last = *d->keyframes.rbegin();
						last->update(object, d->timeline);
					}
				}
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

		for (auto object : d->targetObjects)
		{
			currentKeyFrame->update(object, d->timeline);
		}
	}
}

GM_PRIVATE_OBJECT_UNALIGNED(GMAnimationKeyframe)
{
	GMfloat time;
	GMInterpolationFunctors functors;
};

GM_DEFINE_PROPERTY(GMAnimationKeyframe, GMfloat, Time, time);
GM_DEFINE_PROPERTY(GMAnimationKeyframe, GMInterpolationFunctors, Functors, functors);
GMAnimationKeyframe::GMAnimationKeyframe(GMfloat timePoint)
{
	GM_CREATE_DATA();
	setTime(timePoint);
	setFunctors(GMInterpolationFunctors::getDefaultInterpolationFunctors());
}

GMAnimationKeyframe::~GMAnimationKeyframe()
{

}

GM_PRIVATE_OBJECT_ALIGNED(GMGameObjectKeyframe)
{
	Map<GMGameObject*, GMVec4, std::less<GMGameObject*>, AlignedAllocator<Pair<GMGameObject*, GMVec4>> > translationMap;
	Map<GMGameObject*, GMVec4, std::less<GMGameObject*>, AlignedAllocator<Pair<GMGameObject*, GMVec4>> > scalingMap;
	Map<GMGameObject*, GMQuat, std::less<GMGameObject*>, AlignedAllocator<Pair<GMGameObject*, GMQuat>> > rotationMap;
	GMVec4 translation;
	GMVec4 scaling;
	GMQuat rotation;
	GMfloat timeStart = 0;
	GMint32 component = GMGameObjectKeyframeComponent::NoComponent;
};

GM_DEFINE_PROPERTY(GMGameObjectKeyframe, GMVec4, Translation, translation)
GM_DEFINE_PROPERTY(GMGameObjectKeyframe, GMVec4, Scaling, scaling)
GM_DEFINE_PROPERTY(GMGameObjectKeyframe, GMQuat, Rotation, rotation)
GMGameObjectKeyframe::GMGameObjectKeyframe(
	GMint32 component,
	const GMVec4& translation,
	const GMVec4& scaling,
	const GMQuat& rotation,
	GMfloat timePoint
)
	: GMAnimationKeyframe(timePoint)
{
	GM_CREATE_DATA();

	D(d);
	d->component = component;
	setTranslation(translation);
	setScaling(scaling);
	setRotation(rotation);
}

GMGameObjectKeyframe::~GMGameObjectKeyframe()
{

}

void GMGameObjectKeyframe::reset(IDestroyObject* object)
{
	D(d);
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	gameObj->setTranslation(Translate(d->translationMap[gameObj]));
	gameObj->setScaling(Scale(d->scalingMap[gameObj]));
	gameObj->setRotation(d->rotationMap[gameObj]);
}

void GMGameObjectKeyframe::beginFrame(IDestroyObject* object, GMfloat timeStart)
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

void GMGameObjectKeyframe::endFrame(IDestroyObject* object)
{
	D(d);
	// 去掉误差，把对象放到正确的位置
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	if (d->component != GMGameObjectKeyframeComponent::NoComponent)
	{
		gameObj->beginUpdateTransform();
		if (d->component & GMGameObjectKeyframeComponent::Translate)
			gameObj->setTranslation(Translate(getTranslation()));
		if (d->component & GMGameObjectKeyframeComponent::Scale)
			gameObj->setScaling(Scale(getScaling()));
		if (d->component & GMGameObjectKeyframeComponent::Rotate)
			gameObj->setRotation(getRotation());
		gameObj->endUpdateTransform();
	}
}

void GMGameObjectKeyframe::update(IDestroyObject* object, GMfloat time)
{
	D(d);
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	GMfloat percentage = (time - d->timeStart) / (getTime() - d->timeStart);
	if (percentage > 1.f)
		percentage = 1.f;

	if (d->component != GMGameObjectKeyframeComponent::NoComponent)
	{
		gameObj->beginUpdateTransform();

		const auto& functor = getFunctors();
		if (d->component & GMGameObjectKeyframeComponent::Translate)
			gameObj->setTranslation(Translate(functor.vec4Functor->interpolate(d->translationMap[gameObj], getTranslation(), percentage)));

		if (d->component & GMGameObjectKeyframeComponent::Scale)
			gameObj->setScaling(Scale(functor.vec3Functor->interpolate(d->scalingMap[gameObj], getScaling(), percentage)));

		if (d->component & GMGameObjectKeyframeComponent::Rotate)
			gameObj->setRotation(functor.quatFunctor->interpolate(d->rotationMap[gameObj], getRotation(), percentage));

		gameObj->endUpdateTransform();
	}
}

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT_ALIGNED(GMCameraKeyframe)
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
GM_DEFINE_PROPERTY(GMCameraKeyframe, GMVec3, Position, position)
GM_DEFINE_PROPERTY(GMCameraKeyframe, GMVec3, LookAtDirection, lookAtDirection)
GM_DEFINE_PROPERTY(GMCameraKeyframe, GMVec3, FocusAt, focusAt)

GMCameraKeyframe::GMCameraKeyframe(GMCameraKeyframeComponent component, const GMVec3& position, const GMVec3& lookAtDirectionOrFocusAt, GMfloat timePoint)
	: GMAnimationKeyframe(timePoint)
{
	GM_CREATE_DATA();
	D(d);
	setPosition(position);
	if (component == GMCameraKeyframeComponent::LookAtDirection)
		setLookAtDirection(lookAtDirectionOrFocusAt);
	else if (component == GMCameraKeyframeComponent::FocusAt)
		setFocusAt(lookAtDirectionOrFocusAt);
	else
		gm_warning(gm_dbg_wrap("Wrong component type. This keyframe won't work or work abnormally."));
	d->component = component;
}

GMCameraKeyframe::~GMCameraKeyframe()
{

}

void GMCameraKeyframe::reset(IDestroyObject* object)
{
	D(d);
	GMCamera* camera = gm_cast<GMCamera*>(object);
	GMCameraLookAt lookAt(d->lookAtDirectionMap[camera], d->positionMap[camera]);
	camera->lookAt(lookAt);
}

void GMCameraKeyframe::beginFrame(IDestroyObject* object, GMfloat timeStart)
{
	D(d);
	d->timeStart = timeStart;
	GMCamera* camera = gm_cast<GMCamera*>(object);
	const GMCameraLookAt& lookAt = camera->getLookAt();
	d->positionMap[camera] = lookAt.position;
	d->lookAtDirectionMap[camera] = lookAt.lookDirection;
	d->focusMap[camera] = lookAt.position + lookAt.lookDirection;
}

void GMCameraKeyframe::endFrame(IDestroyObject* object)
{
	D(d);
	GMCamera* camera = gm_cast<GMCamera*>(object);
	GMCameraLookAt lookAt;
	if (d->component == GMCameraKeyframeComponent::LookAtDirection)
	{
		lookAt.position = getPosition();
		lookAt.lookDirection = getLookAtDirection();
	}
	else
	{
		lookAt = GMCameraLookAt::makeLookAt(getPosition(), getFocusAt());
	}
	camera->lookAt(lookAt);
}

void GMCameraKeyframe::update(IDestroyObject* object, GMfloat time)
{
	D(d);
	GMCamera* camera = gm_cast<GMCamera*>(object);
	GMfloat percentage = (time - d->timeStart) / (getTime() - d->timeStart);
	if (percentage > 1.f)
		percentage = 1.f;

	GMCameraLookAt lookAt = camera->getLookAt();
	const auto& functor = getFunctors();
	if (d->component == GMCameraKeyframeComponent::LookAtDirection)
	{
		lookAt.position = functor.vec3Functor->interpolate(d->positionMap[camera], getPosition(), percentage);
		lookAt.lookDirection = SlerpVector(d->lookAtDirectionMap[camera], getLookAtDirection(), percentage);
	}
	else
	{
		lookAt = GMCameraLookAt::makeLookAt(functor.vec3Functor->interpolate(d->positionMap[camera], getPosition(), percentage), functor.vec3Functor->interpolate(d->focusMap[camera], getFocusAt(), percentage));
	}

	camera->lookAt(lookAt);
}

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT_ALIGNED(GMLightKeyframe)
{
	Map<ILight*, GMVec3, std::less<ILight*>, AlignedAllocator<Pair<GMGameObject*, GMVec3>> > ambientMap;
	Map<ILight*, GMVec3, std::less<ILight*>, AlignedAllocator<Pair<GMGameObject*, GMVec3>> > diffuseMap;
	Map<ILight*, GMVec3, std::less<ILight*>, AlignedAllocator<Pair<GMGameObject*, GMVec3>> > positionMap;
	Map<ILight*, GMfloat, std::less<ILight*>> specularMap;
	Map<ILight*, GMfloat, std::less<ILight*>> cutOffMap;
	GMint32 component = GMLightKeyframeComponent::NoComponent;
	GMVec3 ambient;
	GMVec3 diffuse;
	GMfloat specular;
	GMVec3 position;
	GMfloat cutOff;
	GMfloat timeStart = 0;
	const IRenderContext* context = nullptr;
};
GM_DEFINE_PROPERTY(GMLightKeyframe, GMVec3, Ambient, ambient)
GM_DEFINE_PROPERTY(GMLightKeyframe, GMVec3, Diffuse, diffuse)
GM_DEFINE_PROPERTY(GMLightKeyframe, GMfloat, Specular, specular)
GM_DEFINE_PROPERTY(GMLightKeyframe, GMfloat, CutOff, cutOff)
GM_DEFINE_PROPERTY(GMLightKeyframe, GMVec3, Position, position)

GMLightKeyframe::GMLightKeyframe(
	const IRenderContext* context,
	GMint32 component,
	const GMVec3& ambient,
	const GMVec3& diffuse,
	GMfloat specular,
	const GMVec3& position,
	GMfloat cutOff,
	GMfloat timePoint
)
	: GMAnimationKeyframe(timePoint)
{
	GM_CREATE_DATA();
	D(d);
	setAmbient(ambient);
	setDiffuse(diffuse);
	setSpecular(specular);
	setPosition(position);
	setCutOff(cutOff);
	d->component = component;
	d->context = context;
}

GMLightKeyframe::~GMLightKeyframe()
{

}

void GMLightKeyframe::reset(IDestroyObject* object)
{
	D(d);
	ILight* light = gm_cast<ILight*>(object);
	RUN_AND_CHECK(light->setLightAttribute3(GMLight::AmbientIntensity, ValuePointer(d->ambientMap[light])));
	RUN_AND_CHECK(light->setLightAttribute3(GMLight::DiffuseIntensity, ValuePointer(d->diffuseMap[light])));
	RUN_AND_CHECK(light->setLightAttribute(GMLight::SpecularIntensity, d->specularMap[light]));
	RUN_AND_CHECK(light->setLightAttribute3(GMLight::Position, ValuePointer(d->positionMap[light])));
	light->setLightAttribute(GMLight::CutOff, d->cutOffMap[light]);
}

void GMLightKeyframe::beginFrame(IDestroyObject* object, GMfloat timeStart)
{
	D(d);
	d->timeStart = timeStart;
	ILight* light = gm_cast<ILight*>(object);
	if (d->component & GMLightKeyframeComponent::Ambient)
		RUN_AND_CHECK(light->getLightAttribute3(GMLight::AmbientIntensity, ValuePointer(d->ambientMap[light])));
	if (d->component & GMLightKeyframeComponent::Diffuse)
		RUN_AND_CHECK(light->getLightAttribute3(GMLight::DiffuseIntensity, ValuePointer(d->diffuseMap[light])));
	if (d->component & GMLightKeyframeComponent::Specular)
		RUN_AND_CHECK(light->getLightAttribute(GMLight::SpecularIntensity, d->specularMap[light]));
	if (d->component & GMLightKeyframeComponent::Position)
		RUN_AND_CHECK(light->getLightAttribute3(GMLight::Position, ValuePointer(d->positionMap[light])));
	if (d->component & GMLightKeyframeComponent::CutOff)
		light->getLightAttribute(GMLight::CutOff, d->cutOffMap[light]);
}

void GMLightKeyframe::endFrame(IDestroyObject* object)
{
	D(d);
	ILight* light = gm_cast<ILight*>(object);
	RUN_AND_CHECK(light->setLightAttribute3(GMLight::AmbientIntensity, ValuePointer(d->ambient)));
	RUN_AND_CHECK(light->setLightAttribute3(GMLight::DiffuseIntensity, ValuePointer(d->diffuse)));
	RUN_AND_CHECK(light->setLightAttribute(GMLight::SpecularIntensity, d->specular));
	RUN_AND_CHECK(light->setLightAttribute3(GMLight::Position, ValuePointer(d->position)));
	light->setLightAttribute(GMLight::CutOff, d->cutOff);
}

void GMLightKeyframe::update(IDestroyObject* object, GMfloat time)
{
	D(d);
	ILight* light = gm_cast<ILight*>(object);
	GMfloat percentage = (time - d->timeStart) / (getTime() - d->timeStart);
	if (percentage > 1.f)
		percentage = 1.f;

	const auto& functor = getFunctors();
	if (d->component & GMLightKeyframeComponent::Ambient)
	{
		GMVec3 intensity = functor.vec3Functor->interpolate(d->ambientMap[light], getAmbient(), percentage);
		RUN_AND_CHECK(light->setLightAttribute3(GMLight::AmbientIntensity, ValuePointer(intensity)));
	}

	if (d->component & GMLightKeyframeComponent::Diffuse)
	{
		GMVec3 intensity = functor.vec3Functor->interpolate(d->diffuseMap[light], getDiffuse(), percentage);
		RUN_AND_CHECK(light->setLightAttribute3(GMLight::DiffuseIntensity, ValuePointer(intensity)));
	}

	if (d->component & GMLightKeyframeComponent::Specular)
	{
		GMfloat intensity = functor.floatFunctor->interpolate(d->specularMap[light], getSpecular(), percentage);
		RUN_AND_CHECK(light->setLightAttribute(GMLight::SpecularIntensity, intensity));
	}

	if (d->component & GMLightKeyframeComponent::CutOff)
	{
		GMfloat cutOff = functor.floatFunctor->interpolate(d->cutOffMap[light], getCutOff(), percentage);
		RUN_AND_CHECK(light->setLightAttribute(GMLight::CutOff, cutOff));
	}

	if (d->component & GMLightKeyframeComponent::Position)
	{
		GMVec3 position = functor.vec3Functor->interpolate(d->positionMap[light], getPosition(), percentage);
		RUN_AND_CHECK(light->setLightAttribute3(GMLight::Position, ValuePointer(position)));
	}

	if (d->component != GMLightKeyframeComponent::NoComponent)
	{
		GM_ASSERT(d->context);
		d->context->getEngine()->update(GMUpdateDataType::LightChanged);
	}
}

END_NS