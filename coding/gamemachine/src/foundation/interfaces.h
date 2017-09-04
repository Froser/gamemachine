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
};

enum class GMDrawMode
{
	Fill,
	Line,
};

struct IRender
{
	virtual ~IRender() {}
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
	virtual void createWindow(OUT GMUIWindow**) = 0;
	virtual void createGraphicEngine(OUT IGraphicEngine**) = 0;
	virtual void createTexture(AUTORELEASE GMImage*, OUT ITexture**) = 0;
	virtual void createPainter(IGraphicEngine*, GMModel*, OUT GMModelPainter**) = 0;
	virtual void createGamePackage(GMGamePackage*, GamePackageType, OUT IGamePackageHandler**) = 0;
	virtual void createGlyphManager(OUT GMGlyphManager**) = 0;
};
END_NS
#endif