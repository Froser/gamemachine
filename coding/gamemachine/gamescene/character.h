#ifndef __CHARACTER_H__
#define __CHARACTER_H__
#include "common.h"
#include "gameobject.h"
class btMotionState;
class btCollisionShape;

BEGIN_NS
class Character : public GameObject
{
public:
	enum CharacterModelType
	{
		Sphere,
		Box,
	};

public:
	Character(CharacterModelType type, const btTransform& position, btScalar radius);
	Character(CharacterModelType type, const btTransform& position, const btVector3& extents);

public:
	virtual btMotionState* createMotionState() override;
	virtual void drawObject() override;

private:
	virtual btCollisionShape* createCollisionShape() override;

private:
	CharacterModelType m_type;
	btScalar m_radius;
	btVector3 m_extents;
};

END_NS
#endif