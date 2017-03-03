#ifndef __PHYSICSSTRUCT_H__
#define __PHYSICSSTRUCT_H__
#include "common.h"
#include <map>
#include "utilities/vmath.h"
BEGIN_NS

class GameObject;

struct CollisionProperties
{
	vmath::vec3 translation;
	vmath::vec3 velocity;
};

struct CollisionObject
{
	GameObject* object;
	CollisionProperties properties;
};

struct CollisionTree : public std::map<GameObject*, CollisionObject>
{
};

END_NS
#endif