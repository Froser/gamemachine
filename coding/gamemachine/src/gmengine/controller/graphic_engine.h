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

struct GraphicSettings
{
	GMuint fps;
};

typedef std::vector<DrawingItem> DrawingList;
class Camera;
class GameWorld;
class ResourceContainer;
struct CameraLookAt;
struct IGraphicEngine
{
	virtual ~IGraphicEngine();
	virtual void initialize(GameWorld*) = 0;
	virtual void newFrame() = 0;
	virtual void drawObjects(DrawingList& drawingList) = 0;
	virtual void updateCameraView(const CameraLookAt& lookAt) = 0;
	virtual ResourceContainer* getResourceContainer() = 0;
	virtual GraphicSettings& getGraphicSettings() = 0;
};

END_NS
#endif