#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include <vector>
#include "gmengine/controllers/graphic_engine.h"
#include "gameworld.h"
#include "gmengine/controllers/animation.h"
#include "gmdatacore/object.h"
#include "utilities/utilities.h"

BEGIN_NS
struct AnimationMatrices
{
	vmath::mat4 rotation;
	vmath::mat4 tranlation;
	vmath::mat4 scaling;
};

enum AnimationState
{
	AS_Stopped,
	AS_Running,
};

class GameWorld;
struct GameObjectPrivate
{
	GameObjectPrivate()
		: world(nullptr)
		, animationStartTick(0)
		, animationDuration(0)
		, animationState(AS_Stopped)
		, id(0)
	{
	}

	GMuint id;
	AutoPtr<Object> object;
	GameWorld* world;
	Keyframes keyframesRotation;
	Keyframes keyframesTranslation;
	Keyframes keyframesScaling;
	GMint animationStartTick;
	GMint animationDuration;
	AnimationState animationState;
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

//GlyphObject
struct GlyphProperties
{
};

struct GlyphObjectPrivate
{
	std::wstring lastRenderText;
	std::wstring text;
	GlyphProperties properties;
	GMfloat left, bottom, width, height;
	ITexture* texture;
};

class Component;
class GlyphObject : public GameObject
{
	DEFINE_PRIVATE(GlyphObject)

public:
	GlyphObject();

public:
	void setText(const GMWChar* text);
	void setGeometry(GMfloat left, GMfloat bottom, GMfloat width, GMfloat height);
	void updateObject();

private:
	virtual void getReadyForRender(DrawingList& list) override;
	virtual void onAppendingObjectToWorld() override;

private:
	void constructObject();
};

//EntityObject
enum { EntityPlaneNum = 6 };

struct EntityObjectPrivate
{
	vmath::vec3 mins, maxs;
	Plane planes[EntityPlaneNum];
};

class EntityObject : public GameObject
{
	DEFINE_PRIVATE(EntityObject)

public:
	EntityObject(AUTORELEASE Object* obj);

public:
	Plane* getPlanes();
	void getBounds(REF vmath::vec3& mins, REF vmath::vec3& maxs);

private:
	void calc();
	void makePlanes();
};

END_NS
#endif
