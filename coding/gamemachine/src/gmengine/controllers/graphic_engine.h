#ifndef __GRAPHIC_ENGINE_H__
#define __GRAPHIC_ENGINE_H__
#include "common.h"
#include "utilities/vector.h"

BEGIN_NS
class GameObject;
struct DrawingItem
{
	GameObject* gameObject;
};

struct GraphicSettings
{
	GMuint fps;
	GMuint windowSize[2];
	GMuint resolution[2];
	GMuint startPosition[2];
	bool fullscreen;
};

typedef AlignedVector<DrawingItem> DrawingList;
class Camera;
class GameWorld;
class ResourceContainer;
struct CameraLookAt;
struct IGraphicEngine
{
	virtual ~IGraphicEngine();
	virtual void setCurrentWorld(GameWorld*) = 0;
	virtual void newFrame() = 0;
	virtual void drawObjects(DrawingList& drawingList) = 0;
	virtual void updateCameraView(const CameraLookAt& lookAt) = 0;
	virtual ResourceContainer* getResourceContainer() = 0;
	virtual GraphicSettings* getGraphicSettings() = 0;
	virtual void setGraphicSettings(GraphicSettings* settings) = 0;
};

END_NS
#endif