#ifndef __INTERFACES_H__
#define __INTERFACES_H__
#include <defines.h>
#include <glm/fwd.hpp>
#include <gmenums.h>

struct GMFloat4;
struct GMMat4;

BEGIN_NS

// 前置声明
class GameMachine;
class GMGameWorld;
class GMCamera;
class GMGameObject;
class GMGamePackage;
class GMImage;
class GMModel;
class GMGamePackage;
class GMGlyphManager;
class GMShader;
class GMModelDataProxy;
class GMMesh;
class GMTextureSampler;
class GMTextureList;
class GMAssets;
class GMComponent;
class GMWidget;
class GMWidgetResourceManager;
class GMSystemEvent;
class GMCamera;
struct ILight;
struct ISoundPlayer;
struct IGamePackageHandler;
struct IDebugOutput;
struct IAudioPlayer;
struct IGraphicEngine;
struct IRenderer;
struct IInput;
struct IWindow;
struct IRenderContext;
struct GraphicSettings;
struct GMCameraLookAt;
struct GMShaderVariablesDesc;
struct GMShadowSourceDesc;

enum class GameMachineHandlerEvent
{
	FrameStart,
	FrameEnd,
	Simulate,
	Render,
	Activate,
	Deactivate,
	Terminate,
};

enum class GameMachineInterfaceID
{
	GLForwardShaderProgram,
	GLEffectShaderProgram,
	GLDeferredShaderGeometryProgram,
	GLDeferredShaderLightProgram,

	D3D11Device,
	D3D11DeviceContext,
	D3D11SwapChain,
	D3D11DepthStencilView,
	D3D11DepthStencilTexture,
	D3D11RenderTargetView,

	D3D11VertexBuffer,
	D3D11IndexBuffer,
	D3D11ShaderProgram,
	D3D11Effect,

	CustomInterfaceBegin,
	//用户自定义接口须在此之后
};

enum class GMRenderMode
{
	Forward,
	Deferred,
};

enum class GMShaderType
{
	Pixel,
	Vertex,
	Effect,
};

enum class GMRenderEnvironment
{
	Invalid,
	OpenGL,
	DirectX11,
};

//! 模型类型。
/*!
不同的模型类型将采用不同着色器进行处理。自定义的模型类型应当放在CustomStart之后。
*/
enum class GMModelType
{
	Model2D, //! 表示一个2D模型，不考虑其深度来绘制。
	Model3D, //! 表示一个3D模型，是最常见的类型。
	Text, //! 表示一个文字模型，用于绘制文本。
	CubeMap, //! 表示一个立方体贴图。
	Filter, //! 表示一个滤镜，通常是一个四边形模型，在帧缓存中获取纹理来绘制。
	LightPassQuad, //! 表示一个光照传递模型。
	CustomStart, //! 自定义模型类型。将自定义的类型放在此类型后面，匹配自定义的着色器程序。
};

GM_INTERFACE(IGameHandler)
{
	virtual void init(const IRenderContext* context) = 0;
	virtual void start() = 0;
	virtual void event(GameMachineHandlerEvent evt) = 0;
};

GM_INTERFACE(ITexture)
{
	virtual void init() = 0;
	virtual void bindSampler(GMTextureSampler* sampler) = 0;
	virtual void useTexture(GMint textureIndex) = 0;
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

struct GMCursorDesc
{
	GMWindowHandle hWnd;
	GMuint xHotspot;
	GMuint yHotspot;
	GMfloat transparentColor[3];
};

enum class GMUpdateDataType
{
	LightChanged,
	TurnOffCubeMap,
};

//! 当前装载的着色器类型
enum class GMShaderProgramType
{
	DefaultShaderProgram, //!< 当前激活的着色器程序
	ForwardShaderProgram, //!< 正向渲染着色器程序
	DeferredGeometryPassShaderProgram, //!< 延迟渲染Geometry Pass阶段着色器程序
	DeferredLightPassShaderProgram, //!< 延迟渲染Light Pass阶段着色器程序
	FilterShaderProgram, //!< 滤镜着色器程序
};

enum class GMMovement
{
	Move,
	Jump,
};

GM_INTERFACE_FROM(IShaderProgram, IQueriable)
{
	virtual void useProgram() = 0;
	virtual void setMatrix4(const char* name, const GMMat4& value) = 0;
	virtual void setVec4(const char* name, const GMFloat4&) = 0;
	virtual void setVec3(const char* name, const GMfloat value[3]) = 0;
	virtual void setInt(const char* name, GMint value) = 0;
	virtual void setFloat(const char* name, GMfloat value) = 0;
	virtual void setBool(const char* name, bool value) = 0;
	virtual bool setInterfaceInstance(const char* interfaceName, const char* instanceName, GMShaderType type) = 0;
};

// 帧缓存
enum class GMFramebufferFormat
{
	R8G8B8A8_UNORM,
	R32G32B32A32_FLOAT,
};

struct GMFramebuffersDesc
{
	GMRect rect;
	GMfloat clearColor[4] = { 0 };
};

struct GMFramebufferDesc
{
	GMRect rect;
	GMFramebufferFormat framebufferFormat;
};

enum class GMFramebuffersClearType
{
	Color = 0x01,
	Depth = 0x02,
	Stencil = 0x04,
	All = Color | Depth | Stencil,
};

GM_INTERFACE(IFramebuffer)
{
	virtual bool init(const GMFramebufferDesc& desc) = 0;
	virtual ITexture* getTexture() = 0;
	virtual void setName(const GMString& name) = 0;
};

GM_INTERFACE(IFramebuffers)
{
	virtual bool init(const GMFramebuffersDesc& desc) = 0;
	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) = 0;
	virtual GMsize_t count() = 0;
	virtual void use() = 0;
	virtual void bind() = 0;
	virtual void unbind() = 0;
	virtual void clear(GMFramebuffersClearType = GMFramebuffersClearType::All) = 0;
	virtual IFramebuffer* getFramebuffer(GMsize_t) = 0;
	virtual void copyDepthStencilFramebuffer(IFramebuffers* dest) = 0;
};

enum class GMGeometryPassingState
{
	PassingGeometry,
	Done,
};

GM_INTERFACE(IGBuffer)
{
	virtual void init() = 0;
	virtual void geometryPass(const List<GMGameObject*>& objects) = 0;
	virtual void lightPass() = 0;
	virtual IFramebuffers* getGeometryFramebuffers() = 0;
	virtual void setGeometryPassingState(GMGeometryPassingState) = 0;
	virtual GMGeometryPassingState getGeometryPassingState() = 0;
};

GM_INTERFACE(IShaderLoadCallback)
{
	virtual void onLoadShaders(const IRenderContext* context) = 0;
};

enum class GMLightType
{
	Ambient,
	Direct,
};

GM_INTERFACE(ILight)
{
	virtual void setLightPosition(GMfloat position[4]) = 0;
	virtual void setLightColor(GMfloat color[4]) = 0;
	virtual const GMfloat* getLightPosition() const = 0;
	virtual const GMfloat* getLightColor() const = 0;
	virtual void activateLight(GMuint, IRenderer*) = 0;
};

struct GMStencilOptions
{
	enum GMStencilOp
	{
		Equal,
		NotEqual,
		Always
	};

	enum GMStencilWriteMask
	{
		Ox00 = 0x00,
		OxFF = 0xFF,
	};

	GMStencilOptions() = default;
	GMStencilOptions(GMStencilWriteMask mask, GMStencilOp op)
		: writeMask(mask)
		, compareOp(op)
	{
	}

	GMStencilWriteMask writeMask = Ox00;
	GMStencilOp compareOp = Always;
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

	//! 处理GameMachine消息。
	/*!
	  当GameMachine在处理完系统消息之后，此方法将会被调用。<BR>
	  通常用此方法处理一些特殊事件，如窗口大小变化时，需要重新分配帧缓存、G缓存等。
	  \param e GameMachine消息
	  \return 如果此事件被处理，返回true，否则返回false。
	  \sa GameMachineMessage
	*/
	virtual bool event(const GMMessage& e) = 0;

	//! 获取引擎G缓存。
	/*!
	  使用G缓存可以灵活进行延迟渲染。
	  \return 当前的G缓存。
	*/
	virtual IGBuffer* getGBuffer() = 0;

	//! 获取滤镜帧缓存。
	/*!
	  使用滤镜帧缓存，可以为画面渲染各种各样的滤镜。
	  \return 当前滤镜帧缓存。
	*/
	virtual IFramebuffers* getFilterFramebuffers() = 0;

	//! 获取系统默认帧缓存。
	/*!
	  不得对系统默认帧缓存的属性进行更改，只允许对其数据进行修改，如覆盖深度、模板缓存。
	  \return 系统默认帧缓存。
	*/
	virtual IFramebuffers* getDefaultFramebuffers() = 0;

	//! 使用正向绘制对象。
	/*!
	  渲染引擎将先绘制延迟渲染对象，然后再绘制正向渲染对象。
	  \param forwardRenderingObjects 正向渲染对象列表。
	  \param deferredRenderingObjects 延迟渲染对象列表。
	*/
	virtual void draw(const List<GMGameObject*>& forwardRenderingObjects, const List<GMGameObject*>& deferredRenderingObjects) = 0;

	//! 更新绘制数据。
	/*!
	  通知渲染引擎某些状态发生了变化。
	  \param type 需要更新的数据类型。
	*/
	virtual void update(GMUpdateDataType type) = 0;

	//! 增加一个光源。
	/*!
	  将一个光源添加到全局绘制引擎中。<BR>
	  光源的表现行为与着色器程序有关，有些图元可能不会使用到光源，有些图元则可能会。
	  \param light 需要添加的光源。
	*/
	virtual void addLight(AUTORELEASE ILight* light) = 0;

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

	//! 开始进行融合绘制。
	/*!
	  决定下一次调用drawObjects时的混合模式。如果在一个绘制流程中多次调用drawObjects，则应该使用此方法，将本帧的画面和当前帧缓存进行
	融合，否则本帧将会覆盖当前帧缓存已有的所有值。
	  \sa drawObjects(), endBlend()
	*/
	virtual void beginBlend(GMS_BlendFunc sfactor = GMS_BlendFunc::ONE, GMS_BlendFunc dfactor = GMS_BlendFunc::ONE) = 0;

	//! 结束融合绘制。
	/*!
	  结束与当前帧缓存的融合。在这种情况下，执行多次drawObjects，会将其输出的帧缓存覆盖多次。
	  \sa drawObjects(), beginBlend()
	*/
	virtual void endBlend() = 0;

	//! 设置模板缓存属性。
	/*!
	  设置完的模板属性后，模板属性将会应用到所有图形渲染中。
	*/
	virtual void setStencilOptions(const GMStencilOptions& options) = 0;

	//! 获取模板缓存属性。
	/*!
	  查看当前正在应用的模板缓存属性。
	*/
	virtual const GMStencilOptions& getStencilOptions() = 0;

	//! 获取一个着色器程序。
	/*!
	  一个渲染程序一般会装载多个着色器程序，以满足正向渲染、延迟渲染等需求。根据传入的参数返回对应的着色器程序。
	  \param type 着色器程序种类。
	  \return 着色器程序。
	*/
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type = GMShaderProgramType::DefaultShaderProgram) = 0;

	//! 设置一个着色器程序读取的回调。
	/*!
	  当设置引擎准备读取着色器时，此回调接口被调用。一般可以使用setInterface来为引擎添加着色器。
	  \param cb 着色器读取回调接口。
	  \sa IQueriable::setInterface()
	*/
	virtual void setShaderLoadCallback(IShaderLoadCallback* cb) = 0;

	//! 设置一个阴影源。
	/*!
	  阴影源用于显示物体的阴影。
	  \param desc 阴影源的属性。
	*/
	virtual void setShadowSource(const GMShadowSourceDesc& desc) = 0;

	//! 获取某一模型的渲染器。
	/*!
	  不同类型的模型有不同类型的渲染流程，通过使用不同的渲染器来实现不同的绘制效果。
	  \param objectType 模型类型。
	  \return 模型类型对应的渲染器。
	*/
	virtual IRenderer* getRenderer(GMModelType modelType) = 0;

	virtual GMGlyphManager* getGlyphManager() = 0;

	virtual GMCamera& getCamera() = 0;
};

GM_INTERFACE(IRenderer)
{
	virtual void beginModel(GMModel* model, const GMGameObject* parent) = 0;
	virtual void endModel() = 0;
	virtual void draw(GMModel* model) = 0;
};

struct GMWindowStates
{
	GMfloat viewportTopLeftX = 0; //!< 视口左上角X坐标。
	GMfloat viewportTopLeftY = 0; //!< 视口左上角Y坐标。
	GMfloat minDepth = 0; //!< 近平面的深度值。
	GMfloat maxDepth = 1; //!< 远平面的深度值。
	GMString workingAdapterDesc; //!< 适配器信息。
	GMint sampleCount; //!< 多重采样数量。
	GMint sampleQuality; //!< 多重采样质量。
	bool vsyncEnabled = false; //!< 是否垂直同步。
	GMRect renderRect; //!< 当前窗口渲染窗口位置信息。
};

struct IRenderContext
{
	virtual IWindow* getWindow() const = 0;
	virtual IGraphicEngine* getEngine() const = 0;
	virtual void switchToContext() const = 0;
};

GM_INTERFACE_FROM(IWindow, IQueriable)
{
	virtual IInput* getInputMananger() = 0;
	virtual void msgProc(const GMMessage& message) = 0;
	virtual GMWindowHandle create(const GMWindowAttributes& attrs) = 0;
	virtual void centerWindow() = 0;
	virtual void showWindow() = 0;
	virtual GMRect getWindowRect() = 0;
	virtual GMRect getRenderRect() = 0;
	virtual GMWindowHandle getWindowHandle() const = 0;
	virtual bool isWindowActivate() = 0;
	virtual void setWindowCapture(bool capture) = 0;

	virtual bool addWidget(GMWidget* widget) = 0;
	virtual void setHandler(AUTORELEASE IGameHandler* handler) = 0;
	virtual IGameHandler* getHandler() = 0;
	virtual const GMWindowStates& getWindowStates() = 0;
	virtual IGraphicEngine* getGraphicEngine() = 0;
	virtual const IRenderContext* getContext() = 0;
};

GM_INTERFACE(IFactory)
{
	virtual void createWindow(GMInstance instance, OUT IWindow** window) = 0;
	virtual void createTexture(const IRenderContext* context, GMImage*, OUT ITexture**) = 0;
	virtual void createModelDataProxy(const IRenderContext*, GMModel*, OUT GMModelDataProxy**) = 0;
	virtual void createGlyphManager(const IRenderContext* context, OUT GMGlyphManager**) = 0;
	virtual void createFramebuffer(const IRenderContext* context, OUT IFramebuffer**) = 0;
	virtual void createFramebuffers(const IRenderContext* context, OUT IFramebuffers**) = 0;
	virtual void createGBuffer(const IRenderContext* context, OUT IGBuffer**) = 0;
	virtual void createLight(GMLightType, OUT ILight**) = 0;
	virtual void createWhiteTexture(const IRenderContext* context, OUT ITexture**) = 0;
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
