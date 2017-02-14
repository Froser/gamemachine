#ifndef __SCRIPT_ACTIONS_H__
#define __SCRIPT_ACTIONS_H__
#include "common.h"
#include "gmengine/elements/gameobjectprivate.h"
BEGIN_NS

class GameObject;
struct EventItem;
class Action : public IAction
{
public:
	Action(GameObject* sourceObj, EventItem* parent);
	~Action();

public:
	virtual void activate(GameObject* obj) override;
	virtual void finish();

private:
	void resetEventState();

protected:
	GameObject* m_sourceObject;
	GameObject* m_activateObject;
	btTransform m_transform;
	EventItem* m_parentEvent;
};

class Action_Move : public Action
{
public:
	Action_Move(GameObject* sourceObj, EventItem* parent, GMint duration, btVector3& to);

	virtual void handleAction() override;

private:
	GMint m_duration;
	btVector3 m_to;
};

END_NS
#endif