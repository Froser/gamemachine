#ifndef __INTERFACES_H__
#define __INTERFACES_H__
#include <defines.h>
#include <glm/fwd.hpp>
#include <gmenums.h>

struct GMFloat4;
struct GMMat4;

// Audio
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
	GMWord    wFormatTag;
	GMWord    nChannels;
	GMDWord   nSamplesPerSec;
	GMDWord   nAvgBytesPerSec;
	GMWord    nBlockAlign;
	GMWord    wBitsPerSample;
	GMWord    cbSize;
} WAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

BEGIN_NS

// 前置声明
class GameMachine;
class GMGameWorld;
class GMCamera;
class GMGameObject;
class GMGamePackage;
class GMImage;
class GMScene;
class GMModel;
class GMGamePackage;
class GMGlyphManager;
class GMShader;
class GMModelDataProxy;
class GMPart;
class GMTextureSampler;
class GMTextureList;
class GMAssets;
class GMComponent;
class GMWidget;
class GMWidgetResourceManager;
class GMSystemEvent;
class GMCamera;
class GMRenderTechniqueManager;
class GMRenderTechniques;
class GMPrimitiveManager;
struct ILight;
struct ISoundPlayer;
struct IGamePackageHandler;
struct IDebugOutput;
struct IAudioPlayer;
struct IGraphicEngine;
struct ITechnique;
struct IInput;
struct IWindow;
struct IRenderContext;
struct GraphicSettings;
struct GMCameraLookAt;
struct GMShadowSourceDesc;

class GMAsset;
typedef GMAsset GMModelAsset;
typedef GMAsset GMSceneAsset;
typedef GMAsset GMPhysicsShapeAsset;
typedef GMAsset GMTextureAsset;

enum class GameMachineHandlerEvent
{
	FrameStart,
	FrameEnd,
	Update,
	Render,
	Activate,
	Deactivate,
	Terminate,
};

enum class GameMachineInterfaceID
{
	GLForwardShaderProgram,
	GLFiltersShaderProgram,
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

	CSMFramebuffer,

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
	Unknown,
	Pixel,
	Vertex,
	Geometry,
	Effect, //DirectX11
};

enum class GMRenderEnvironment
{
	Invalid,
	OpenGL,
	DirectX11,
	EndOfRenderEnvironment,
};

//! 模型类型。
/*!
  不同的模型类型将采用不同着色器进行处理。
*/
enum class GMModelType
{
	Model2D, //! 表示一个2D模型，不考虑其深度来绘制。
	Model3D, //! 表示一个3D模型，是最常见的类型。
	Text, //! 表示一个文字模型，用于绘制文本。
	CubeMap, //! 表示一个立方体贴图。
	Filter, //! 表示一个滤镜，通常是一个四边形模型，在帧缓存中获取纹理来绘制。
	LightPassQuad, //! 表示一个光照传递模型。
	Particle, //! 表示一个粒子模型。
	Custom, //! 自定义模型类型。
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
	virtual void useTexture(GMint32 textureIndex) = 0;
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
struct GMWindowDesc
{
	bool createNewWindow = true; //!< 是否创建一个新窗口，或者是使用已经存在的窗口来渲染
	GMWindowHandle existWindowHandle = NULL;

	GMWindowHandle hwndParent = NULL;
	GMString windowName = L"Default GameMachine Main Window";
	GMDWord dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME;
	GMDWord dwExStyle = NULL;
	GMRectF rc = { 0, 0, 0, 0 }; //!< 渲染窗口大小。如果需要创建新窗口，且宽度或高度小于或等于0，则分别拿到一个默认值。如果不需要创建窗口，且宽度或高度小于或等于0，则拿hwndParent窗口大小。
	GMint32 samples = 8;
	GMInstance instance;
};
#else
typedef GMint32 GMWindowHandle;
typedef GMint32 GMInstance;
struct GMWindowDesc
{
	bool createNewWindow = true; //!< 是否创建一个新窗口，或者是使用已经存在的窗口来渲染
	GMWindowHandle existWindowHandle = 0;

	GMWindowHandle hwndParent = 0;
	GMString windowName = L"Default GameMachine Main Window";
	GMulong dwStyle = 0;
	GMulong dwExStyle = 0;
	GMRectF rc = { 0, 0, 1024, 768 };
	GMint32 samples = 8;
	GMInstance instance;
};
#endif

struct GMCursorDesc
{
	GMWindowHandle hWnd;
	GMuint32 xHotspot;
	GMuint32 yHotspot;
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

GM_INTERFACE_FROM(IShaderProgram, IQueriable)
{
	virtual void useProgram() = 0;
	virtual GMint32 getIndex(const GMString& name) = 0;
	virtual void setMatrix4(GMint32 index, const GMMat4& value) = 0;
	virtual void setVec4(GMint32 index, const GMFloat4&) = 0;
	virtual void setVec3(GMint32 index, const GMfloat value[3]) = 0;
	virtual void setInt(GMint32 index, GMint32 value) = 0;
	virtual void setFloat(GMint32 index, GMfloat value) = 0;
	virtual void setBool(GMint32 index, bool value) = 0;
	virtual bool setInterfaceInstance(const GMString& interfaceName, const GMString& instanceName, GMShaderType type) = 0;
};

enum class GMComputeBufferType
{
	Structured, //!< 结构化缓存。它从CPU设置，并被GPU读取。
	UnorderedStructured, //!< 无序访问的结构化缓存。它从GPU设置，不能从CPU写入。用来获取计算结果。
	Constant, //!< 常量缓存。一般当作传入计算着色器的参数。
};

#if GM_USE_DX11
typedef IUnknown *GMComputeHandle;
#else
typedef void *GMComputeHandle;
#endif
typedef GMComputeHandle GMComputeBufferHandle;
typedef GMComputeHandle GMComputeSRVHandle;
typedef GMComputeHandle GMComputeUAVHandle;

//! GameMachine计算着色器。所有GPGPU相关计算的实例，由此类创建。
/*!
  正常情况下，可以从GameMachine的getFactory()中获取一个IComputeShaderProgram实例。
*/
GM_INTERFACE_FROM(IComputeShaderProgram, IQueriable)
{
	//! 触发并行计算。
	/*!
	  着色器装载后、所有缓存设置好之后，可以开始触发计算。<BR>
	  此方法是同步的，在计算着色器完成计算之前，此线程将会被阻塞。
	  \param threadGroupCountX 工作组第1维的数量。
	  \param threadGroupCountY 工作组第2维的数量。
	  \param threadGroupCountZ 工作组第3维的数量。
	*/
	virtual void dispatch(GMint32 threadGroupCountX, GMint32 threadGroupCountY, GMint32 threadGroupCountZ) = 0;

	//! 读取一个计算着色器代码。
	/*!
	  如果一个着色器代码来自内存，那么path建议设置维"."。
	  \param path 着色器代码路径。
	  \param source 着色器代码内容。
	  \param entryPoint 着色器代码入口。
	*/
	virtual void load(const GMString& path, const GMString& source, const GMString& entryPoint) = 0;

	//! 从一个现有的缓存中创建一个新的缓存。
	/*!
	  新缓存会有旧缓存相同的大小和类型，但是内容为空，且是只读的。
	  \param bufferSrc 缓存来源。
	  \param bufferOut 新创建的缓存。
	  \return 是否创建成功。
	*/
	virtual bool createReadOnlyBufferFrom(GMComputeBufferHandle bufferSrc, OUT GMComputeBufferHandle* bufferOut) = 0;

	//! 创建一个缓存。
	/*!
	  缓存是CPU和GPU的桥梁。我们需要将数据放入缓存中，然后将缓存提交给GPU来进行计算。
	  \param elementSize 缓存中一个元素的大小。
	  \param count 缓存中元素的数量。
	  \param initData 缓存数据。
	  \param type 缓存类型。
	  \param bufOut 创建的缓存。
	  \return 是否创建成功。
	*/
	virtual bool createBuffer(GMuint32 elementSize, GMuint32 count, void* initData, GMComputeBufferType type, OUT GMComputeBufferHandle* bufOut) = 0;

	//! 为一个有序的结构化缓存创建一个着色器缓存资源视图
	/*!
	  每当创建了一个缓存，都需要为缓存创建资源视图，这样缓存才能被使用。<BR>
	  \param buf 需要创建视图的缓存。buf的类型一定要为GMComputeBufferType::Structured，否则结果是未定义的。
	  \param srv 创建的资源视图。
	  \return 是否创建成功。
	  \sa GMComputeBufferType
	*/
	virtual bool createBufferShaderResourceView(GMComputeBufferHandle buf, OUT GMComputeSRVHandle* srv) = 0;

	//! 为一个无序的结构化缓存创建一个着色器缓存资源视图
	/*!
	  每当创建了一个缓存，都需要为缓存创建资源视图，这样缓存才能被使用。<BR>
	  \param buf 需要创建视图的缓存。buf的类型一定要为GMComputeBufferType::UnorderedStructured，否则结果是未定义的。
	  \param srv 创建的资源视图。
	  \return 是否创建成功。
	  \sa GMComputeBufferType
	*/
	virtual bool createBufferUnorderedAccessView(GMComputeBufferHandle buf, OUT GMComputeUAVHandle* uav) = 0;

	//! 绑定一个着色器资源视图。
	/*!
	  着色器的绑定入口将会和调用此方法的顺序保持一致。<BR>
	  对于OpenGL，绑定点的编号(binding=n)和bindUnorderedAccessView、bindConstantBuffer共享，每次调用之后编号递增1。<BR>
	  对于DirectX，绑定点的编号是独立的，如调用5次后，它表示的寄存器是register(t5)。<BR>
	  \param num 绑定的个数。
	  \param handles 绑定的有序访问结构化缓存视图。
	  \sa bindUnorderedAccessView(), bindConstantBuffer()
	*/
	virtual void bindShaderResourceView(GMuint32 num, GMComputeSRVHandle* handles) = 0;

	//! 绑定一个着色器资源视图。
	/*!
	  着色器的绑定入口将会和调用此方法的顺序保持一致。<BR>
	  对于OpenGL，绑定点的编号(binding=n)和bindShaderResourceView、bindConstantBuffer共享，每次调用之后编号递增1。<BR>
	  对于DirectX，绑定点的编号是独立的，如调用5次后，它表示的寄存器是register(u5)。<BR>
	  \param num 绑定的个数。
	  \param handles 绑定的无序访问结构化缓存视图。
	  \sa bindShaderResourceView(), bindConstantBuffer()
	*/
	virtual void bindUnorderedAccessView(GMuint32 num, GMComputeUAVHandle* handles) = 0;

	//! 绑定一个着色器资源视图。
	/*!
	  着色器的绑定入口将会和调用此方法的顺序保持一致。<BR>
	  对于OpenGL，绑定点的编号(binding=n)和bindShaderResourceView、bindUnorderedAccessView共享，每次调用之后编号递增1。<BR>
	  对于DirectX，绑定点的编号是独立的，如调用5次后，它表示的寄存器是register(b5)。<BR>
	  \param num 绑定的个数。
	  \param handles 绑定的常量缓存。
	  \sa bindShaderResourceView(), bindUnorderedAccessView()
	*/
	virtual void bindConstantBuffer(GMComputeBufferHandle handle) = 0;

	//! 为一个缓存设置数据。
	/*!
	  为一个缓存设置一段数据。
	  \param handle 待设置的缓存。
	  \param type 指定的缓存的类型，它一定要和缓存本身的类型一致，否则结果是未定义的。
	  \param data 待设置的数据。
	  \param sizeInBytes 待设置的数据大小，单位为字节。
	  \sa GMComputeBufferType
	*/
	virtual void setBuffer(GMComputeBufferHandle handle, GMComputeBufferType type, void* data, GMuint32 sizeInBytes) = 0;

	//! 拷贝一段缓存。
	/*!
	  \param dest 目标缓存。
	  \param src 来源缓存。
	*/
	virtual void copyBuffer(GMComputeBufferHandle dest, GMComputeBufferHandle src) = 0;

	//! 将缓存映射到一段内存中。
	/*!
	  某些缓存不能被CPU读取，在使用此方法前，先使用canRead()判断一下。
	  \param handle 待映射缓存。
	  \return 映射结果。
	  \sa canRead
	*/
	virtual void* mapBuffer(GMComputeBufferHandle handle) = 0;

	//! 结束一段映射。
	/*!
	  映射结束后，此端缓存将会被提交并更新到GPU。
	  \param handle 待映射缓存。
	  \return 映射结果。
	*/
	virtual void unmapBuffer(GMComputeBufferHandle handle) = 0;

	//! 返回某个缓存是否能被CPU读取。
	/*!
	  \param handle 被判断的缓存。
	  \return 是否能被CPU读取。
	*/
	virtual bool canRead(GMComputeBufferHandle handle) = 0;

	//! 返回某个缓存的大小，单位为字节。
	/*!
	  \param type 指定该缓存类型，一定要和缓存原本的类型一致，否则结果是未定义的。
	  \param handle 被判断的缓存。它必须和type指定的类型是一致的。
	  \return 缓存大小。
	  \sa GMComputeBufferType
	*/
	virtual GMsize_t getBufferSize(GMComputeBufferType type, GMComputeBufferHandle handle) = 0;

	//! 释放一个着色器资源。
	/*!
	  着色器资源包括缓存、缓存资源视图、无序缓存资源视图。<BR>
	  在资源使用完后一定要释放，否则会造成内存泄漏。
	  \param handle 待释放资源。
	*/
	virtual void release(GMComputeHandle handle) = 0;
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
	virtual void getTexture(REF GMTextureAsset&) = 0;
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
	virtual void setClearColor(const GMfloat rgba[4]) = 0;
};

typedef GMint32 GMCascadeLevel;
GM_INTERFACE(ICSMFramebuffers)
{
	virtual void setShadowSource(const GMShadowSourceDesc&) = 0;
	virtual GMCascadeLevel cascadedBegin() = 0;
	virtual GMCascadeLevel cascadedEnd() = 0;
	virtual void applyCascadedLevel(GMCascadeLevel) = 0;
	virtual GMCascadeLevel currentLevel() = 0;
	virtual GMfloat getEndClip(GMCascadeLevel) = 0;
	virtual void setEachCascadeEndClip(GMCascadeLevel) = 0;
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

struct IShaderLoadCallback
{
	virtual void onLoadShaders(const IRenderContext* context) = 0;
};

enum class GMLightType
{
	PointLight,
	DirectionalLight,
	Spotlight,
};

GM_INTERFACE(ILight)
{
	typedef GMint32 GMLightAttribute;

	virtual bool setLightAttribute3(GMLightAttribute, GMfloat[3]) = 0;
	virtual bool setLightAttribute(GMLightAttribute, GMfloat) = 0;
	virtual void activateLight(GMuint32 index, ITechnique* technique) = 0;
};

struct GMStencilOptions
{
	enum GMStencilFunction
	{
		Equal,
		NotEqual,
		Always,
		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		Never,
	};

	enum GMStencilWriteMask
	{
		Ox00 = 0x00,
		OxFF = 0xFF,
	};

	enum GMStencilOp
	{
		Keep,
		Zero,
		Replace,
	};

	GMStencilOptions() = default;
	GMStencilOptions(
		GMStencilWriteMask writemask,
		GMStencilFunction func,
		GMStencilOp stencilFailedOperation = Keep,
		GMStencilOp stencilDepthFailedOperation = Keep,
		GMStencilOp stencilPassOperation = Replace
	)
		: writeMask(writemask)
		, compareFunc(func)
		, stencilFailedOp(stencilFailedOperation)
		, stencilDepthFailedOp(stencilDepthFailedOperation)
		, stencilPassOp(stencilPassOperation)
	{
	}

	GMStencilWriteMask writeMask = Ox00;
	GMStencilFunction compareFunc = Always;
	GMStencilOp stencilFailedOp = Keep;
	GMStencilOp stencilDepthFailedOp = Keep;
	GMStencilOp stencilPassOp = Replace;
};

typedef GMsize_t GMLightIndex;

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
	virtual bool msgProc(const GMMessage& e) = 0;

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
	  \return 光源的索引。
	*/
	virtual GMLightIndex addLight(AUTORELEASE ILight* light) = 0;

	//! 获取一个光源。
	/*!
	  根据索引返回一个光源。如果索引非法，返回一个空指针。
	  \return 目标光源。
	*/
	virtual ILight* getLight(GMLightIndex index) = 0;

	//! 移除所有光源。
	/*!
	  移除引擎中的所有光源。
	*/
	virtual void removeLights() = 0;

	//! 开始进行融合绘制。
	/*!
	  决定下一次调用drawObjects时的混合模式。如果在一个绘制流程中多次调用drawObjects，则应该使用此方法，将本帧的画面和当前帧缓存进行
	融合，否则本帧将会覆盖当前帧缓存已有的所有值。
	  \sa drawObjects(), endBlend()
	*/
	virtual void beginBlend(
		GMS_BlendFunc sfactorRGB = GMS_BlendFunc::One,
		GMS_BlendFunc dfactorRGB = GMS_BlendFunc::One,
		GMS_BlendOp opRGB = GMS_BlendOp::Add,
		GMS_BlendFunc sfactorAlpha = GMS_BlendFunc::One,
		GMS_BlendFunc dfactorAlpha = GMS_BlendFunc::One,
		GMS_BlendOp opAlpha = GMS_BlendOp::Add
	) = 0;

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
	virtual ITechnique* getTechnique(GMModelType modelType) = 0;

	virtual GMGlyphManager* getGlyphManager() = 0;

	virtual GMCamera& getCamera() = 0;

	virtual void setCamera(const GMCamera&) = 0;

	virtual GMRenderTechniqueManager* getRenderTechniqueManager() = 0;

	virtual GMPrimitiveManager* getPrimitiveManager() = 0;

	//! 初始化一个对象数据代理。
	/*!
	初始化一个对象数据代理。每当新建一个GMModel时，需要调用此方法，为GMModel设置一个数据代理。此方法将用工厂类来为GMModel创建一个GMModelDataProxy对象。<br>
	在创建之后，数据代理马上会将GMModel顶点数据传输到GPU。
	\param context 当前渲染上下文。
	\param model 需要创建数据代理的模型对象。创建好的GMModelDataProxy会绑定在此对象上。在此对象析构时，GMModelDataProxy也会析构，用户不需要关心它的生命周期。
	\param transfer 是否马上传递顶点信息到GPU。传递后，对象的顶点信息将在内存中被释放。
	*/
	virtual void createModelDataProxy(const IRenderContext* context, GMModel* model, bool transfer = true) = 0;
};

GM_INTERFACE(ITechnique)
{
	virtual void beginScene(GMScene* scene) = 0;
	virtual void endScene() = 0;
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
	GMint32 sampleCount; //!< 多重采样数量。
	GMint32 sampleQuality; //!< 多重采样质量。
	bool vsyncEnabled = false; //!< 是否垂直同步。
	GMRect renderRect; //!< 当前窗口渲染窗口位置信息。
	GMRect framebufferRect; //!< 帧缓存矩形区域。它不随窗口大小而改变。它表示默认帧缓存的区域。
};

GM_INTERFACE(IRenderContext)
{
	virtual IWindow* getWindow() const = 0;
	virtual IGraphicEngine* getEngine() const = 0;

	/*!
	  切换渲染上下文到当前上下文。它会切换默认的帧缓存，更改渲染引擎上下文。<BR>
	  每当开始渲染一个窗口时，它将会被调用。
	*/
	virtual void switchToContext() const = 0;
};

enum class GMCursorType
{
	Arrow,
	IBeam,
	Wait,
	Cross,
	UpArrow,
	Hand,
	EndOfEnum,
};

typedef GMLResult (GM_SYSTEM_CALLBACK *GMWindowProcHandler)(GMWindowHandle hWnd, GMuint32 uMsg, GMWParam wParam, GMLParam lParam);
GM_INTERFACE_FROM(IWindow, IQueriable)
{
	virtual IInput* getInputManager() = 0;
	virtual void msgProc(const GMMessage& message) = 0;
	virtual GMWindowHandle create(const GMWindowDesc& desc) = 0;
	virtual void centerWindow() = 0;
	virtual void showWindow() = 0;
	virtual GMRect getWindowRect() = 0;
	virtual GMRect getRenderRect() = 0;
	virtual GMRect getFramebufferRect() = 0;
	virtual GMWindowHandle getWindowHandle() const = 0;
	virtual bool isWindowActivate() = 0;
	virtual void setWindowCapture(bool capture) = 0;
	virtual bool addWidget(GMWidget* widget) = 0;
	virtual void setHandler(AUTORELEASE IGameHandler* handler) = 0;
	virtual IGameHandler* getHandler() = 0;
	virtual const GMWindowStates& getWindowStates() = 0;
	virtual IGraphicEngine* getGraphicEngine() = 0;
	virtual const IRenderContext* getContext() = 0;
	virtual void setCursor(GMCursorType type) = 0;
	virtual GMWindowProcHandler getProcHandler() = 0;
};

GM_INTERFACE(IFactory)
{
	virtual void createWindow(GMInstance instance, IWindow* parent, OUT IWindow** window) = 0;
	virtual void createTexture(const IRenderContext* context, GMImage*, REF GMTextureAsset&) = 0;
	virtual void createModelDataProxy(const IRenderContext*, GMModel*, OUT GMModelDataProxy**) = 0;
	virtual void createGlyphManager(const IRenderContext* context, OUT GMGlyphManager**) = 0;
	virtual void createFramebuffer(const IRenderContext* context, OUT IFramebuffer**) = 0;
	virtual void createFramebuffers(const IRenderContext* context, OUT IFramebuffers**) = 0;
	virtual void createShadowFramebuffers(const IRenderContext* context, OUT IFramebuffers**) = 0;
	virtual void createGBuffer(const IRenderContext* context, OUT IGBuffer**) = 0;
	virtual void createLight(GMLightType, OUT ILight**) = 0;
	virtual void createWhiteTexture(const IRenderContext* context, REF GMTextureAsset&) = 0;
	virtual void createEmptyTexture(const IRenderContext* context, REF GMTextureAsset&) = 0;
	virtual void createShaderPrograms(const IRenderContext* context, const GMRenderTechniqueManager& manager, REF Vector<IShaderProgram*>* out) = 0;
	virtual bool createComputeShaderProgram(const IRenderContext* context, OUT IComputeShaderProgram** out) = 0;
};

typedef WAVEFORMATEX GMWaveFormatEx;

struct GMAudioFileInfo
{
	GMint32 format;
	const void* data;
	GMsize_t size;
	GMint32 frequency;
	GMWaveFormatEx waveFormatExHeader;
};

GM_INTERFACE(IAudioStream)
{
	virtual GMuint32 getBufferSize() = 0; // 每个部分的buffer大小
	virtual GMuint32 getBufferNum() = 0; // buffer一共分为多少部分
	virtual bool readBuffer(GMbyte* data) = 0;
	virtual void nextChunk(gm::GMlong chunkNum) = 0;
	virtual void rewind() = 0;
};

GM_INTERFACE(IAudioFile)
{
	virtual bool isStream() = 0;
	virtual IAudioStream* getStream() = 0;
	virtual const GMAudioFileInfo& getFileInfo() = 0;
	virtual GMuint32 getBufferId() = 0;
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
