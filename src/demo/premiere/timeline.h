#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include <gamemachine.h>
#include <gmxml.h>
#include <gmanimation.h>
#include <extensions/objects/gmwavegameobject.h>
#include <gmgraphicengine.h>
#include <gmdiscretedynamicsworld.h>

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
	Shadow,
	PhysicsWorld,
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

template <typename T>
struct AlignCache : AlignedVector<T>
{
	GMsize_t put(const T& value)
	{
		this->push_back(value);
		return this->size() - 1;
	}

	GMsize_t put(T&& value)
	{
		this->push_back(std::move(value));
		return this->size() - 1;
	}
};



class Slots
{
public:
	Slots(Slots* parent);
	~Slots();

public:
	Slots* getParent();
	bool addSlot(const GMString& name, const GMString& value);
	bool getSlotByName(const GMString& name, GMString& result);
	void destroy();

private:
	void append(Slots*);
	void remove(Slots*);
	
private:
	HashMap<GMString, GMString, GMStringHashFunctor> m_slots;
	Slots* m_parent;
	List<Slots*> m_children;
};

enum Warning
{
	Warning_ObjectExistsWarning = 1,

	Warning_WarningsEnd,
};

class SupressedWarnings
{
public:
	SupressedWarnings();

	void set(Warning);
	bool isSet(Warning);
	void clear(Warning);

private:
	Array<GMint32, Warning_WarningsEnd> m_data;
};

class SlotsStack
{
public:
	SlotsStack();

public:
	void pushSlots();
	void popSlots();
	bool addSlot(const GMString& name, const GMString& value);
	bool getSlotByName(const GMString& name, GMString& result);

private:
	GMOwnedPtr<Slots> m_root;
	Slots* m_currentSlots;
};

class Scanner;
class Timeline
{
	friend class Scanner;
	using ShaderCallback = std::function<void(GMShader&)>;

public:
	Timeline(const IRenderContext* context, GMGameWorld* world);
	~Timeline();

public:
	// 解析一个Timeline文件
	bool parse(const GMString& timelineContent);
	bool parseComponent(const GMString& componentContent);
	void update(GMDuration dt);
	void play();
	void pause();

private:
	void initPresetConstants();
	void parseElements(GMXMLElement*);
	void parseDefines(GMXMLElement*);
	void parseAssets(GMXMLElement*);
	void parseObjects(GMXMLElement*);
	void parseActions(GMXMLElement*);
	void parseInclude(GMXMLElement*);
	void parseComponent(GMXMLElement*);
	GMint32 parseCameraAction(GMXMLElement*, REF CameraParams& cp, REF GMCameraLookAt& lookAt);
	void parseParticlesAsset(GMXMLElement*);
	void parseParticlesObject(GMXMLElement*);
	void parseCocos2DParticleAttributes(IParticleSystem*, GMXMLElement*);
	void interpolateCamera(GMXMLElement*, GMfloat);
	void interpolateLight(GMXMLElement*, ILight*, GMfloat);
	void interpolateObject(GMXMLElement*, GMGameObject*, GMfloat);
	AnimationContainer& getAnimationFromObject(AssetType, void*);
	GMAsset findAsset(const GMString& assetName);
	const PBR* findPBR(const GMString& assetName);
	GMBuffer findBuffer(const GMString& bufferName);
	void parseTransform(GMGameObject*, GMXMLElement*);
	void parseTextures(GMGameObject*, GMXMLElement*);
	void parseMaterial(GMGameObject*, GMXMLElement*);
	void parseBlend(GMGameObject*, GMXMLElement*);
	void parsePrimitiveAttributes(GMGameObject*, GMXMLElement*);
	void parsePhysics(GMDiscreteDynamicsWorld*, GMXMLElement*);
	void parsePhysics(GMGameObject*, GMXMLElement*);
	void parsePhysicsAttributes(GMRigidPhysicsObject*, GMXMLElement*);
	ShaderCallback parseTexture(GMXMLElement*, const char* type, GMTextureType textureType);
	ShaderCallback parseTextureTransform(GMXMLElement*, const char* type, GMTextureType textureType, GMS_TextureTransformType transformType);
	ShaderCallback parseTexturePBR(GMXMLElement*);
	ShaderCallback parseMaterial(GMXMLElement*);
	void parseAttributes(GMGameObject*, GMXMLElement*, Action&);
	void parseWaveObjectAttributes(GMWaveGameObjectDescription&, GMXMLElement*);
	void parseWaveAttributes(GMWaveDescription&, GMXMLElement*);
	void addObject(AutoReleasePtr<GMGameObject>*, GMXMLElement*, Action&);
	void addObject(AutoReleasePtr<ILight>*, GMXMLElement*, Action&);
	void addObject(AutoReleasePtr<IParticleSystem>*, GMXMLElement*, Action&);
	void addObject(AutoReleasePtr<GMPhysicsWorld>*, GMXMLElement*, Action&);
	void addObject(GMShadowSourceDesc*, GMXMLElement*, Action&);
	void removeObject(ILight*, GMXMLElement*, Action&);
	void removeObject(GMGameObject*, GMXMLElement*, Action&, bool);
	void setColorForModel(GMModel*, const GMfloat color[4]);
	void transferColorForModel(GMModel*, const GMfloat color[4]);
	CurveType parseCurve(GMXMLElement*, GMInterpolationFunctors&);
	void bindAction(const Action& a);
	void runImmediateActions();
	void runActions();
	AssetType getAssetType(const GMString& objectName, OUT void** out);
	void playAudio(IAudioSource* source);
	void play(GMGameObject*, const GMString&);
	GMString getValueFromDefines(GMString id);
	GMint32 parseInt(const GMString& str, bool* ok = nullptr);
	GMfloat parseFloat(const GMString& str, bool* ok = nullptr);
	bool objectExists(const GMString& id);

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
	HashMap<GMString, AutoReleasePtr<GMPhysicsWorld>, GMStringHashFunctor> m_physicsWorld;
	HashMap<GMString, GMShadowSourceDesc, GMStringHashFunctor> m_shadows;
	HashMap<GMString, GMString, GMStringHashFunctor> m_defines;
	HashMap<GMString, GMString, GMStringHashFunctor> m_presetConstants;
	Map<GMModel*, GMVertices> m_verticesCache;
	std::multiset<Action> m_immediateActions;
	std::multiset<Action> m_deferredActions;
	std::multiset<Action>::iterator m_currentAction;
	Vector<Map<IDestroyObject*, AnimationContainer>> m_animations;
	AlignCache<GMCameraLookAt> m_cameraLookAtCache;
	bool m_playing;
	bool m_finished;
	GMDuration m_timeline;
	GMDuration m_lastTime;
	GMDuration m_checkpointTime;
	IAudioReader* m_audioReader;
	IAudioPlayer* m_audioPlayer;
	SlotsStack m_slots;
	SupressedWarnings m_supressedWarnings;
};

class Scanner
{
public:
	explicit Scanner(const GMString& line, Timeline& timeline);

public:
	bool nextInt(REF GMint32& value);
	bool nextFloat(REF GMfloat& value);

private:
	Timeline& m_timeline;
	GMScanner m_scanner;
};

#endif