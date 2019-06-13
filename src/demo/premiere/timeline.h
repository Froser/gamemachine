#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include <gamemachine.h>
#include <gmxml.h>
#include <gmanimation.h>

using namespace gm;

struct CameraParams;
struct Action
{
	enum RunType
	{
		Immediate,
		Deferred,
	};

	RunType runType;
	GMfloat timePoint;
	std::function<void()> action;
};

enum class CurveType
{
	NoCurve,
	CubicBezier,
};

enum class AssetType
{
	NotFound,
	Camera,
	GameObject,
	Light,
};

inline bool operator<(const Action& lhs, const Action& rhs) GM_NOEXCEPT
{
	return lhs.timePoint < rhs.timePoint;
}

class AnimationContainer
{
public:
	AnimationContainer();

public:
	void newAnimation();
	void nextAnimation();
	void playAnimation();
	void pauseAnimation();
	void updateAnimation(GMfloat dt);
	GMAnimation& currentAnimation();

	void setCurrentAnimationTimePoint(GMfloat timePoint);
	GMfloat getCurrentAnimationTimePoint();
	GMfloat getTimePointFromStart(GMfloat timePoint);

private:
	GMsize_t m_animationCount;
	GMsize_t m_currentAnimationIndex;
	Vector<GMAnimation> m_animations;
	Vector<GMfloat> m_animationTimePoints;
};

class Timeline
{
public:
	Timeline(const IRenderContext* context, GMGameWorld* world);

public:
	// 解析一个Timeline文件
	bool parse(const GMString& timelineContent);
	void update(GMDuration dt);
	void play();
	void pause();

private:
	void parseElements(GMXMLElement*);
	void parseAssets(GMXMLElement*);
	void parseObjects(GMXMLElement*);
	void parseActions(GMXMLElement*);
	GMint32 parseCameraAction(GMXMLElement*, REF CameraParams& cp, REF GMCameraLookAt& lookAt);

private:
	void interpolateCamera(GMXMLElement*, Action&, GMfloat);
	void interpolateLight(GMXMLElement*, Action&, ILight*, GMfloat);
	AnimationContainer& getAnimationFromObject(AssetType, void*);

private:
	GMAsset findAsset(const GMString& assetName);
	GMBuffer findBuffer(const GMString& bufferName);
	void parseTransform(GMGameObject*, GMXMLElement*);
	void parseTextures(GMGameObject*, GMXMLElement*);
	void parseMaterial(GMGameObject*, GMXMLElement*);
	void parseAttributes(GMGameObject*, GMXMLElement*, Action&);
	void addObject(GMGameObject*, GMXMLElement*, Action&);
	void addObject(ILight*, GMXMLElement*, Action&);
	void removeObject(ILight*, GMXMLElement*, Action&);
	void removeObject(GMGameObject*, GMXMLElement*, Action&);
	CurveType parseCurve(GMXMLElement*, GMInterpolationFunctors&);

	void bindAction(const Action& a);
	void runImmediateActions();
	void runActions();

	AssetType getAssetType(const GMString& objectName, OUT void** out);

private:
	const IRenderContext* m_context;
	GMGameWorld* m_world;
	HashMap<GMString, GMBuffer, GMStringHashFunctor> m_buffers;
	HashMap<GMString, GMAsset, GMStringHashFunctor> m_assets;
	HashMap<GMString, GMGameObject*, GMStringHashFunctor> m_objects;
	HashMap<GMString, ILight*, GMStringHashFunctor> m_lights;
	std::multiset<Action> m_immediateActions;
	std::multiset<Action> m_deferredActions;
	std::multiset<Action>::iterator m_currentAction;
	Vector<Map<IDestroyObject*, AnimationContainer>> m_animations;
	bool m_playing;
	bool m_finished;
	GMDuration m_timeline;
	GMDuration m_lastTime;
};

#endif