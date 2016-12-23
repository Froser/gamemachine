#ifndef __GRAPHIC_ENGINE_H__
#define __GRAPHIC_ENGINE_H__
#include "common.h"
BEGIN_NS

class GameObject;
class Camera;
struct IGraphicEngine
{
	virtual ~IGraphicEngine();
	virtual void drawObject(GMfloat transformMatrix[16], GameObject* obj) = 0;
	virtual void updateCameraView(Camera& camera) = 0;
};

END_NS
#endif