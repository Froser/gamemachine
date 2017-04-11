#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include "gameobjectprivate.h"
#include <vector>
#include "gmengine/controllers/graphic_engine.h"
#include "gameworld.h"

BEGIN_NS
struct AnimationMatrices
{
	vmath::mat4 rotation;
	vmath::mat4 tranlation;
	vmath::mat4 scaling;
};

class GameWorld;
class GameObject
{
	DEFINE_PRIVATE(GameObject)

public:
	GameObject(AUTORELEASE Object* obj);
	virtual ~GameObject() {};

public:
	void setObject(AUTORELEASE Object* obj);
	Object* getObject();

	virtual void setWorld(GameWorld* world);
	GameWorld* getWorld();

	Keyframes& getKeyframesRotation();
	Keyframes& getKeyframesTranslation();
	Keyframes& getKeyframesScaling();
	void startAnimation(GMuint duration);
	void stopAnimation();

public:
	virtual void getReadyForRender(DrawingList& list);
	virtual void onAppendingObjectToWorld();

protected:
	virtual AnimationMatrices getAnimationMatrix();
	virtual vmath::mat4 getTransformMatrix(GMfloat glTrans[16]);
};

END_NS
#endif