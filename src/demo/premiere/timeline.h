#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include <gamemachine.h>
#include <tinyxml2.h>
#include <gmanimation.h>

using namespace gm;

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

inline bool operator<(const Action& lhs, const Action& rhs) GM_NOEXCEPT
{
	return lhs.timePoint < rhs.timePoint;
}

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
	void parseElements(tinyxml2::XMLElement*);
	void parseAssets(tinyxml2::XMLElement*);
	void parseObjects(tinyxml2::XMLElement*);
	void parseActions(tinyxml2::XMLElement*);

private:
	void parseTransform(GMGameObject*, tinyxml2::XMLElement*);

	void bindAction(const Action& a);
	void runImmediateActions();
	void runActions();

private:
	const IRenderContext* m_context;
	GMGameWorld* m_world;
	HashMap<GMString, GMAsset, GMStringHashFunctor> m_assets;
	HashMap<GMString, GMGameObject*, GMStringHashFunctor> m_objects;
	HashMap<GMString, ILight*, GMStringHashFunctor> m_lights;
	std::multiset<Action> m_actions;
	std::multiset<Action>::iterator m_currentAction;
	Vector<GMAnimation> m_animations;
	bool m_playing;
	bool m_finished;
	GMDuration m_timeline;
};

#endif