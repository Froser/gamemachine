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
	AudioSource,
};

inline bool operator<(const Action& lhs, const Action& rhs) GM_NOEXCEPT
{
	return lhs.timePoint < rhs.timePoint;
}

struct PBR
{
	GMTextureAsset albedo, metallicRoughnessAO, normal;
};

class AnimationContainer
{
public:
	AnimationContainer();

public:
	void playAnimation();
	void pauseAnimation();
	void updateAnimation(GMfloat dt);
	GMAnimation& currentEditingAnimation();
	void nextEditAnimation();
	void nextPlayingAnimation();

private:
	void newAnimation();

private:
	Vector<GMAnimation> m_animations;
	GMint32 m_playingAnimationIndex;
	GMint32 m_editingAnimationIndex;
};

class Timeline
{
public:
	Timeline(const IRenderContext* context, GMGameWorld* world);
	~Timeline();

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
	void parseInclude(GMXMLElement*);
	GMint32 parseCameraAction(GMXMLElement*, REF CameraParams& cp, REF GMCameraLookAt& lookAt);

private:
	void interpolateCamera(GMXMLElement*, GMfloat);
	void interpolateLight(GMXMLElement*, ILight*, GMfloat);
	void interpolateObject(GMXMLElement*, GMGameObject*, GMfloat);
	AnimationContainer& getAnimationFromObject(AssetType, void*);

private:
	GMAsset findAsset(const GMString& assetName);
	const PBR* findPBR(const GMString& assetName);
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
	void playAudio(IAudioSource* source);

private:
	const IRenderContext* m_context;
	GMGameWorld* m_world;
	HashMap<GMString, GMBuffer, GMStringHashFunctor> m_buffers;
	HashMap<GMString, GMAsset, GMStringHashFunctor> m_assets;
	HashMap<GMString, PBR, GMStringHashFunctor> m_pbrs;
	HashMap<GMString, GMGameObject*, GMStringHashFunctor> m_objects;
	HashMap<GMString, ILight*, GMStringHashFunctor> m_lights;
	HashMap<GMString, IAudioFile*, GMStringHashFunctor> m_audioFiles;
	HashMap<GMString, IAudioSource*, GMStringHashFunctor> m_audioSources;
	std::multiset<Action> m_immediateActions;
	std::multiset<Action> m_deferredActions;
	std::multiset<Action>::iterator m_currentAction;
	Vector<Map<IDestroyObject*, AnimationContainer>> m_animations;
	bool m_playing;
	bool m_finished;
	GMDuration m_timeline;
	GMDuration m_lastTime;
	IAudioReader* m_audioReader;
	IAudioPlayer* m_audioPlayer;
};

#endif