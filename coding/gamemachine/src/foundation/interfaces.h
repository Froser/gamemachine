#ifndef __INTERFACES_H__
#define __INTERFACES_H__
BEGIN_NS

// 前置声明
class GameMachine;
class GMGameWorld;
class GMCamera;
class GMGameObject;
class DrawingList;
class GMGamePackage;
class GMImage;
class GMModel;
class GMGamePackage;
class GMGlyphManager;
class GMUIWindow;
class Shader;
class GMModelPainter;
class GMMesh;
class GMTextureFrames;
class GMTexture;
class GMResourceContainer;
struct ISoundPlayer;
struct IGamePackageHandler;
struct GraphicSettings;
struct CameraLookAt;
struct IDebugOutput;

namespace linear_math
{
	class Matrix4x4;
}

enum class GameMachineEvent
{
	FrameStart,
	FrameEnd,
	Simulate,
	Render,
	Activate,
	Deactivate,
	Terminate,
};

enum class GameMachineMessageType
{
	Quit,
	Console,
	WindowSizeChanged,
};

struct GameMachineMessage
{
	GameMachineMessageType msgType;
	GMint type;
	GMint value;
};

GM_INTERFACE(IGameHandler)
{
	virtual void init() = 0;
	virtual void start() = 0;
	virtual void event(GameMachineEvent evt) = 0;
	virtual bool isWindowActivate() = 0;
};

GM_INTERFACE(ITexture)
{
	virtual void drawTexture(GMTextureFrames* frames) = 0;
};

class GMLight;
GM_INTERFACE(IGraphicEngine)
{
	virtual void start() = 0;
	virtual void newFrame() = 0;
	virtual bool event(const GameMachineMessage& e) = 0;
	virtual void drawObjects(GMGameObject *objects[], GMuint count) = 0;
	virtual void updateCameraView(const CameraLookAt& lookAt) = 0;
	virtual void addLight(const GMLight& light) = 0;
	virtual void beginCreateStencil() = 0;
	virtual void endCreateStencil() = 0;
	virtual void beginUseStencil(bool inverse) = 0;
	virtual void endUseStencil() = 0;
	virtual void beginBlend() = 0;
	virtual void endBlend() = 0;
	virtual void beginFullRendering() = 0;
	virtual void endFullRendering() = 0;
};

enum class GMDrawMode
{
	Fill,
	Line,
};

GM_INTERFACE(IRender)
{
	virtual void begin(IGraphicEngine* engine, GMMesh* mesh, GMfloat* modelTransform) = 0;
	virtual void beginShader(Shader& shader, GMDrawMode mode) = 0;
	virtual void endShader() = 0;
	virtual void end() = 0;
	virtual void updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt) = 0;
	virtual void activateLights(const GMLight* lights, GMint count) = 0;
};

enum GamePackageType
{
	GPT_DIRECTORY,
	GPT_ZIP,
};

GM_INTERFACE(IFactory)
{
	virtual void createGraphicEngine(OUT IGraphicEngine**) = 0;
	virtual void createTexture(AUTORELEASE GMImage*, OUT ITexture**) = 0;
	virtual void createPainter(IGraphicEngine*, GMModel*, OUT GMModelPainter**) = 0;
	virtual void createGamePackage(GMGamePackage*, GamePackageType, OUT IGamePackageHandler**) = 0;
	virtual void createGlyphManager(OUT GMGlyphManager**) = 0;
};

#if _WINDOWS
typedef HINSTANCE GMInstance;
typedef HWND GMWindowHandle;
struct GMWindowAttributes
{
	HWND hwndParent;
	LPCTSTR pstrName;
	DWORD dwStyle;
	DWORD dwExStyle;
	RECT rc;
	HMENU hMenu;
	GMInstance instance;
};
#else
typedef GMuint GMUIInstance;
typedef GMuint GMWindowHandle;
struct GMUIWindowAttributes {};
#endif

GM_INTERFACE(IWindow)
{
	virtual void update() = 0;
	virtual void swapBuffers() const = 0;
	virtual gm::GMWindowHandle create(const GMWindowAttributes& attrs) = 0;
	virtual void centerWindow() = 0;
	virtual void showWindow() = 0;
	virtual bool handleMessage() = 0;
	virtual GMRect getWindowRect() = 0;
	virtual GMRect getClientRect() = 0;
	virtual GMWindowHandle getWindowHandle() const = 0;
	virtual bool event(const GameMachineMessage& msg) = 0;
};

GM_ALIGNED_STRUCT(GMConsoleHandle)
{
	IWindow* window;
	IDebugOutput* dbgoutput;
};

END_NS
#endif