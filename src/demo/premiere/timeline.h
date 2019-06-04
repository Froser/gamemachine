#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include <gamemachine.h>
#include <tinyxml2.h>
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
	const IRenderContext* m_context;
	GMGameWorld* m_world;
	HashMap<GMString, GMAsset, GMStringHashFunctor> m_assets;
	HashMap<GMString, GMGameObject*, GMStringHashFunctor> m_objects;
	HashMap<GMString, ILight*, GMStringHashFunctor> m_lights;
	std::multiset<Action> m_actions;
};

#endif