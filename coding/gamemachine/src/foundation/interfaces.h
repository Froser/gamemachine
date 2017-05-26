#ifndef __INTERFACES_H__
#define __INTERFACES_H__
BEGIN_NS

// 前置声明
class GameMachine;
class GameWorld;
class Camera;
class ResourceContainer;
class GameObject;
class DrawingList;
class GamePackage;
struct GraphicSettings;
struct TextureFrames;
struct CameraLookAt;

struct DrawingItem
{
	GameObject* gameObject;
};

struct IWindow
{
	virtual ~IWindow() {}
	virtual bool createWindow() = 0;
	virtual GMRect getWindowRect() = 0;
	virtual bool handleMessages() = 0;
	virtual void swapBuffers() = 0;
#ifdef _WINDOWS
	virtual HWND hwnd() = 0;
#endif
};

enum GameMachineEvent
{
	GM_EVENT_SIMULATE,
	GM_EVENT_RENDER,
	GM_EVENT_ACTIVATE,
};

struct IGameHandler
{
	virtual ~IGameHandler() {}
	virtual void setGameMachine(GameMachine* gm) = 0;
	virtual void init() = 0;
	virtual void event(GameMachineEvent evt) = 0;
	virtual bool isWindowActivate() = 0;
};

struct ITexture
{
	virtual ~ITexture() {}
	virtual void drawTexture(TextureFrames* frames) = 0;
};

struct IGraphicEngine
{
	virtual ~IGraphicEngine() {}
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