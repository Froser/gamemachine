#ifndef __INTERFACES_H__
#define __INTERFACES_H__
#include <glm/fwd.hpp>
#include <input.h>
#include <gmenums.h>

struct GMFloat4;

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
class GMShader;
class GMModelPainter;
class GMMesh;
class GMTextureFrames;
class GMTexture;
class GMAssets;
struct ISoundPlayer;
struct IGamePackageHandler;
struct GraphicSettings;
struct GMCameraLookAt;
struct IDebugOutput;
struct IAudioPlayer;

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
	None,
	Quit,
	CrashDown,
	Console,
	WindowSizeChanged,
	Dx11Ready,
};

enum class GameMachineInterfaceID
{
	GLForwardShaderProgram,
	GLEffectShaderProgram,
	GLDeferredShaderGeometryProgram,
	GLDeferredShaderLightProgram,

	D3D11Device,
	D3D11DeviceContext,
	DXGISwapChain,
	D3D11DepthStencilView,
	D3D11RenderTargetView,

	D3D11VertexShader,
	D3D11VertexShaderBuffer,
	D3D11PixelShader,
	D3D11PixelShaderBuffer,

	CustomInterfaceBegin,
	//用户自定义接口须在此之后
};

enum GMShaderType
{
	GM_FRAGMENT_SHADER = 0x8B30,
	GM_PIXEL_SHADER = GM_FRAGMENT_SHADER, // 同义
	GM_VERTEX_SHADER = 0x8B31
};

struct GameMachineMessage
{
	GameMachineMessage(GameMachineMessageType t = GameMachineMessageType::None, GMint tp = 0, GMint v = 0, void* objPtr = nullptr)
		: msgType(t)
		, type(tp)
		, value(v)
	{}

	GameMachineMessageType msgType = GameMachineMessageType::None;
	GMint type = 0;
	GMint value = 0;
	void* objPtr = 0;
};

GM_INTERFACE(IGameHandler)
{
	virtual void init() = 0;
	virtual void start() = 0;
	virtual void event(GameMachineEvent evt) = 0;
};

GM_INTERFACE(ITexture)
{
	virtual void drawTexture(GMTextureFrames* frames) = 0;
};

//! 可以获取、设置对象的数据接口。
/*!
  如果某类继承此接口，表示此类可以通过getInterface和setInterface来获取、设置接口。
*/
GM_INTERFACE(IQueriable)
{
	//! 从接口实例中获取某个对象。
	/*!
	  如果获取成功（即接口实例识别对象类型和对象），返回true，否则返回false。<br>
	  需要注意的是，如果获取的是一个COM对象，一定要用GMComPtr来接。否则，用户需要自己释放此COM对象的一个引用计数。
	  \param id 对象类型。
	  \param out 获取对象的指针。
	  \return 是否获取成功。
	*/
	virtual bool getInterface(GameMachineInterfaceID id, void** out) = 0;

	//! 将一个对象设置进本接口的实例中。
	/*!
	  如果设置成功（即接口实例识别对象类型和对象），返回true，否则返回false。
	  \param id 对象类型。
	  \param in 需要设置的对象指针。
	  \return 是否设置成功。
	*/
	virtual bool setInterface(GameMachineInterfaceID id, void* in) = 0;
};

enum class GMBufferMode
{
	Normal,
	NoFramebuffer,
};

enum class GMUpdateDataType
{
	ProjectionMatrix,
	ViewMatrix,
	TurnOffCubeMap,
};

//! 当前装载的着色器类型
enum class GMShaderProgramType
{
	CurrentShaderProgram, //!< 当前激活的着色器程序
	ForwardShaderProgram, //!< 正向渲染着色器程序
	DeferredGeometryPassShaderProgram, //!< 延迟渲染Geometry Pass阶段着色器程序
	DeferredLightPassShaderProgram, //!< 延迟渲染Light Pass阶段着色器程序
};

enum class GMMovement
{
	Move,
	Jump,
};

GM_INTERFACE(IShaderProgram)
{
	virtual void useProgram() = 0;
	virtual void setMatrix4(const char* name, const GMfloat value[16]) = 0;
	virtual void setVec4(const char* name, const GMFloat4&) = 0;
	virtual void setVec3(const char* name, const GMfloat value[3]) = 0;
	virtual void setInt(const char* name, GMint value) = 0;
	virtual void setFloat(const char* name, GMfloat value) = 0;
	virtual void setBool(const char* name, bool value) = 0;
};

class GMLight;
struct IGraphicEngine;
GM_INTERFACE(IShaderLoadCallback)
{
	virtual void onLoadShaders(IGraphicEngine* engine) = 0;
};

//! 图形绘制引擎接口
/*!
  提供最基本的绘制功能。
*/
GM_INTERFACE_FROM(IGraphicEngine, IQueriable)
{
	//! 初始化绘制引擎。
	/*!
	  该方法将在GameMachine初始化时被调用。
	*/
	virtual void init() = 0;

	//! 刷新帧缓存，新建一帧。
	/*!
	  此方法保证默认帧缓存将会被清空，但是并不保证其他帧缓存（如G缓存）被清空。<BR>
	  如果要清空其它帧缓存，应该调用需要清除的帧缓存的响应方法。<BR>
	  此方法将会清除默认帧缓存中的颜色缓存、深度缓存和模板缓存。
	*/
	virtual void newFrame() = 0;

	//! 处理GameMachine消息。
	/*!
	  当GameMachine在处理完系统消息之后，此方法将会被调用。<BR>
	  通常用此方法处理一些特殊事件，如窗口大小变化时，需要重新分配帧缓存、G缓存等。
	  \param e GameMachine消息
	  \return 如果此事件被处理，返回true，否则返回false。
	  \sa GameMachineMessage
	*/
	virtual bool event(const GameMachineMessage& e) = 0;

	//! 绘制若干个GMGameObject对象
	/*!
	  绘制GMGameObject对象。这个方法会将绘制好的图元到目标缓存，目标缓存取决于GMBufferMode的值。
	  \param objects 待绘制的对象。
	  \param count 待绘制对象的个数。
	  \param bufferMode 绘制模式。如果模式为GMBufferMode::Normal，程序将按照正常流程绘制，如果模式为GMBufferMode::NoFramebuffer，程序会将绘制结果直接保存在默认帧缓存上，而不会保存在其他帧缓存中。
	*/
	virtual void drawObjects(GMGameObject *objects[], GMuint count, GMBufferMode bufferMode = GMBufferMode::Normal) = 0;

	//! 更新绘制数据。
	/*!
	  调用此方法会将数据更新到绘制的着色器程序中。
	  \param type 需要更新的数据类型。
	*/
	virtual void update(GMUpdateDataType type) = 0;

	//! 增加一个光源。
	/*!
	  将一个光源添加到全局绘制引擎中。<BR>
	  光源的表现行为与着色器程序有关，有些图元可能不会使用到光源，有些图元则可能会。
	  \param light 需要添加的光源。
	*/
	virtual void addLight(const GMLight& light) = 0;

	//! 移除所有光源。
	/*!
	  移除引擎中的所有光源。
	*/
	virtual void removeLights() = 0;

	//! 清除当前激活帧缓存下的模板缓存。
	/*!
	  如果当前激活缓存是默认帧缓存，则清除默认帧缓存中的模板缓存。<BR>
	  如果当前激活的是其它帧缓存（如G缓存等），则清除它的模板缓存。
	*/
	virtual void clearStencil() = 0;

	//! 开始创建模板缓存。
	/*!
	  在当前激活帧缓存下创建模板缓存。<BR>
	  在此方法被执行后，所有的绘制将会被写入进被激活的帧缓存的模板缓存中。
	  \sa endCreateStencil()
	*/
	virtual void beginCreateStencil() = 0;

	//! 结束创建模板缓存。
	/*!
	  结束模板缓存的创建。<BR>
	  在此方法被执行后，所有的绘制不会被写入到当前激活的帧缓存的模板缓存中。
	  \sa beginCreateStencil()
	*/
	virtual void endCreateStencil() = 0;

	//* 开始使用帧缓存。
	/*!
	  在此方法被执行后，绘制图元将会根据当前激活的帧缓存的模板缓存进行绘制。
	  \param inverse 表示图元是绘制在模板中，还是模板外。
	  \sa endUseStencil()
	*/
	virtual void beginUseStencil(bool inverse) = 0;

	//! 结束使用帧缓存。
	/*!
	  在此方法被执行后，图元的绘制将不会依据任何模板缓存。
	  \sa beginUseStencil()
	*/
	virtual void endUseStencil() = 0;

	//! 开始进行融合绘制
	/*!
	  决定下一次调用drawObjects时的混合模式。如果在一个绘制流程中多次调用drawObjects，则应该使用此方法，将本帧的画面和当前帧缓存进行
	融合，否则本帧将会覆盖当前帧缓存已有的所有值。
	  \sa drawObjects(), endBlend()
	*/
	virtual void beginBlend(GMS_BlendFunc sfactor = GMS_BlendFunc::ONE, GMS_BlendFunc dfactor = GMS_BlendFunc::ONE) = 0;

	//! 结束融合绘制
	/*!
	  结束与当前帧缓存的融合。在这种情况下，执行多次drawObjects，会将其输出的帧缓存覆盖多次。
	  \sa drawObjects(), beginBlend()
	*/
	virtual void endBlend() = 0;

	//! 获取一个着色器程序
	/*!
	  一个渲染程序一般会装载多个着色器程序，以满足正向渲染、延迟渲染等需求。根据传入的参数返回对应的着色器程序。
	  \param type 着色器程序种类。
	  \return 着色器程序。
	*/
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type = GMShaderProgramType::CurrentShaderProgram) = 0;

	//! 设置一个着色器程序读取的回调
	/*!
	  当设置引擎准备读取着色器时，此回调接口被调用。一般可以使用setInterface来为引擎添加着色器。
	  \param cb 着色器读取回调接口。
	  \sa IQueriable::setInterface()
	*/
	virtual void setShaderLoadCallback(IShaderLoadCallback* cb) = 0;
};

class GMComponent;
GM_INTERFACE(IRenderer)
{
	virtual void beginModel(GMModel* model, const GMGameObject* parent) = 0;
	virtual void endModel() = 0;
	virtual void beginComponent(GMComponent* component) = 0;
	virtual void endComponent() = 0;
};

GM_INTERFACE(IFactory)
{
	virtual void createGraphicEngine(OUT IGraphicEngine**) = 0;
	virtual void createTexture(GMImage*, OUT ITexture**) = 0;
	virtual void createPainter(IGraphicEngine*, GMModel*, OUT GMModelPainter**) = 0;
	virtual void createGlyphManager(OUT GMGlyphManager**) = 0;
};

#if GM_WINDOWS
typedef HINSTANCE GMInstance;
typedef HWND GMWindowHandle;
struct GMWindowAttributes
{
	GMWindowHandle hwndParent = NULL;
	GMString windowName = L"Default GameMachine Main Window";
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME;
	DWORD dwExStyle = NULL;
	RECT rc = { 0, 0, 1024, 768 };
	HMENU hMenu = NULL;
	GMint samples = 8;
	GMInstance instance;
};
#else
struct GMWindowAttributes
{
};
typedef GMuint GMUIInstance;
typedef GMuint GMWindowHandle;
#endif

GM_INTERFACE_FROM(IWindow, IQueriable)
{
	virtual IInput* getInputMananger() = 0;
	virtual void update() = 0;
	virtual gm::GMWindowHandle create(const GMWindowAttributes& attrs) = 0;
	virtual void centerWindow() = 0;
	virtual void showWindow() = 0;
	//! 处理新一轮消息循环。
	/*!
	  此方法由GameMachine调用。处理一轮消息循环。
	  \return 是否应该继续消息循环。如果返回false，则程序退出。
	*/
	virtual bool handleMessage() = 0;
	virtual GMRect getWindowRect() = 0;
	virtual GMRect getClientRect() = 0;
	virtual GMWindowHandle getWindowHandle() const = 0;
	virtual bool event(const GameMachineMessage& msg) = 0;
	virtual bool isWindowActivate() = 0;
	virtual void setLockWindow(bool lock) = 0;
};

GM_ALIGNED_STRUCT(GMConsoleHandle)
{
	IWindow* window = nullptr;
	IDebugOutput* dbgoutput;
};

// Audio
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
	WORD    wFormatTag;
	WORD    nChannels;
	DWORD   nSamplesPerSec;
	DWORD   nAvgBytesPerSec;
	WORD    nBlockAlign;
	WORD    wBitsPerSample;
	WORD    cbSize;
} WAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

struct GMAudioFileInfo
{
	GMint format;
	const void* data;
	GMint size;
	GMint frequency;
	WAVEFORMATEX waveFormatExHeader;
};

GM_INTERFACE(IAudioStream)
{
	virtual GMuint getBufferSize() = 0; // 每个部分的buffer大小
	virtual GMuint getBufferNum() = 0; // buffer一共分为多少部分
	virtual bool readBuffer(GMbyte* data) = 0;
	virtual void nextChunk(gm::GMlong chunkNum) = 0;
	virtual void rewind() = 0;
};

GM_INTERFACE(IAudioFile)
{
	virtual bool isStream() = 0;
	virtual IAudioStream* getStream() = 0;
	virtual const GMAudioFileInfo& getFileInfo() = 0;
	virtual GMuint getBufferId() = 0;
};

GM_INTERFACE(IAudioSource)
{
	virtual void play(bool loop) = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual void rewind() = 0;
};

GM_INTERFACE(IAudioPlayer)
{
	virtual void createPlayerSource(IAudioFile*, OUT gm::IAudioSource** handle) = 0;
};

GM_INTERFACE(IAudioReader)
{
	virtual bool load(GMBuffer& buffer, OUT gm::IAudioFile** f) = 0;
};

END_NS
#endif
