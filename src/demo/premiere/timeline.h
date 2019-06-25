#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include <gamemachine.h>
#include <gmxml.h>
#include <gmanimation.h>

using namespace gm;

template <typename T>
class AutoReleasePtr
{
public:
	AutoReleasePtr()
		: m_ptr(nullptr)
		, m_released(false)
	{

	}

	AutoReleasePtr(T* ptr)
		: m_ptr(ptr)
		, m_released(false)
	{

	}

	AutoReleasePtr(AutoReleasePtr&& rhs)
	{
		*this = std::move(rhs);
	}

	~AutoReleasePtr()
	{
		if (!m_released && m_ptr)
			GM_delete(m_ptr);
	}

	AutoReleasePtr& operator=(AutoReleasePtr&& rhs)
	{
		using namespace std;
		swap(m_released, rhs.m_released);
		swap(m_ptr, rhs.m_ptr);
		return *this;
	}

	T* release()
	{
		if (!m_released)
			m_released = true;
		return m_ptr;
	}

	T* get() GM_NOEXCEPT
	{
		return m_ptr;
	}

	T* operator->() GM_NOEXCEPT
	{
		return get();
	}

	bool operator!() GM_NOEXCEPT
	{
		return !!get();
	}

private:
	bool m_released;
	T* m_ptr;
};

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
	Particles,
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
	void parseParticlesAsset(GMXMLElement*);
	void parseParticlesObject(GMXMLElement*);
	void parseCocos2DParticleAttributes(IParticleSystem*, GMXMLElement*);

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
	void addObject(AutoReleasePtr<GMGameObject>*, GMXMLElement*, Action&);
	void addObject(AutoReleasePtr<ILight>*, GMXMLElement*, Action&);
	void addObject(AutoReleasePtr<IParticleSystem>*, GMXMLElement*, Action&);
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
	HashMap<GMString, AutoReleasePtr<GMGameObject>, GMStringHashFunctor> m_objects;
	HashMap<GMString, AutoReleasePtr<ILight>, GMStringHashFunctor> m_lights;
	HashMap<GMString, AutoReleasePtr<IAudioFile>, GMStringHashFunctor> m_audioFiles;
	HashMap<GMString, AutoReleasePtr<IAudioSource>, GMStringHashFunctor> m_audioSources;
	HashMap<GMString, AutoReleasePtr<IParticleSystem>, GMStringHashFunctor> m_particleSystems;
	std::multiset<Action> m_immediateActions;
	std::multiset<Action> m_deferredActions;
	std::multiset<Action>::iterator m_currentAction;
	Vector<Map<IDestroyObject*, AnimationContainer>> m_animations;
	bool m_playing;
	bool m_finished;
	GMDuration m_timeline;
	GMDuration m_lastTime;
	GMDuration m_checkpointTime;
	IAudioReader* m_audioReader;
	IAudioPlayer* m_audioPlayer;
};

#endif