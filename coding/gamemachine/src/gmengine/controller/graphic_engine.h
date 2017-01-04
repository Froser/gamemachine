#ifndef __GRAPHIC_ENGINE_H__
#define __GRAPHIC_ENGINE_H__
#include "common.h"
#include <vector>

BEGIN_NS
class GameObject;
struct DrawingItem
{
	GMfloat trans[16];
	GameObject* gameObject;
};

typedef std::vector<DrawingItem> DrawingList;
class Camera;
class GameWorld;
struct ILightController;
class ResourceContainer;
struct IGraphicEngine
{
	virtual ~IGraphicEngine();
	virtual void initialize(GameWorld*) = 0;
	virtual void newFrame() = 0;
	virtual void drawObjects(DrawingList& drawingList) = 0;
	virtual void updateCameraView(Camera& camera) = 0;
	virtual ILightController& getLightController() = 0;
	virtual ResourceContainer* getResourceContainer() = 0;
};

END_NS
#endif