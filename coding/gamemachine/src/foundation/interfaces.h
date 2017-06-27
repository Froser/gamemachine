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
class GMGamePackage;
class GMImage;
class Object;
class ObjectPainter;
class GMGamePackage;
class GMGlyphManager;
class GMUIWindow;
struct ISoundPlayer;
struct IGamePackageHandler;
struct GraphicSettings;
struct TextureFrames;
struct CameraLookAt;

enum class GameMachineEvent
{
	FrameStart,
	FrameEnd,
	Simulate,
	Render,
	Activate,
	Deactivate,
};

GM_INTERFACE(IGameHandler)
{
	virtual void start() = 0;
	virtual void event(GameMachineEvent evt) = 0;
	virtual bool isWindowActivate() = 0;
};

GM_INTERFACE(ITexture)
{
	virtual void drawTexture(TextureFrames* frames) = 0;
};

GM_INTERFACE(IGraphicEngine)
{
	virtual void start() = 0;
	virtual void setCurrentWorld(GMGameWorld*) = 0;
	virtual void newFrame() = 0;
	virtual void setViewport(const GMRect& rect) = 0;
	virtual void drawObject(GMGameObject* obj) = 0;
	virtual void updateCameraView(const CameraLookAt& lookAt) = 0;
	virtual ResourceContainer* getResourceContainer() = 0;
};

enum GamePackageType
{
	GPT_DIRECTORY,
	GPT_ZIP,
};

GM_INTERFACE(IFactory)
{
	virtual void createWindow(OUT GMUIWindow**) = 0;
	virtual void createGraphicEngine(OUT IGraphicEngine**) = 0;
	virtual void createTexture(AUTORELEASE GMImage*, OUT ITexture**) = 0;
	virtual void createPainter(IGraphicEngine*, Object*, OUT ObjectPainter**) = 0;
	virtual void createGamePackage(GMGamePackage*, GamePackageType, OUT IGamePackageHandler**) = 0;
	virtual void createGlyphManager(OUT GMGlyphManager**) = 0;
};
END_NS
#endif