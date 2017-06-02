#ifndef __INTERFACES_H__
#define __INTERFACES_H__
BEGIN_NS

// 前置声明
class GameMachine;
class GMGameWorld;
class Camera;
class ResourceContainer;
class GMGameObject;
class DrawingList;
class GamePackage;
class Image;
class Object;
class ObjectPainter;
class GamePackage;
class GlyphManager;
struct IWindow;
struct ISoundPlayer;
struct IGamePackageHandler;
struct GraphicSettings;
struct TextureFrames;
struct CameraLookAt;

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
	virtual void setCurrentWorld(GMGameWorld*) = 0;
	virtual void newFrame() = 0;
	virtual void drawObject(GMGameObject* obj) = 0;
	virtual void updateCameraView(const CameraLookAt& lookAt) = 0;
	virtual ResourceContainer* getResourceContainer() = 0;
	virtual GraphicSettings* getGraphicSettings() = 0;
	virtual void setGraphicSettings(GraphicSettings* settings) = 0;
};

enum GamePackageType
{
	GPT_DIRECTORY,
	GPT_ZIP,
};

struct IFactory
{
	virtual ~IFactory() {};
	virtual void createWindow(OUT IWindow**) = 0;
	virtual void createGraphicEngine(OUT IGraphicEngine**) = 0;
	virtual void createTexture(AUTORELEASE Image*, OUT ITexture**) = 0;
	virtual void createPainter(IGraphicEngine*, Object*, OUT ObjectPainter**) = 0;
	virtual void createGamePackage(GamePackage*, GamePackageType, OUT IGamePackageHandler**) = 0;
	virtual void createGlyphManager(OUT GlyphManager**) = 0;
};

END_NS
#endif