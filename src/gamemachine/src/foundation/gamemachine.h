#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <gminput.h>
#include <gmassets.h>
#include <gmglyphmanager.h>
#include <gmgamepackage.h>
#include <gmcamera.h>
#include <gmthread.h>
#include <gmmessage.h>

BEGIN_NS

extern "C"
{
	enum class GMCapability
	{
		SupportOpenGL,
		SupportDirectX11,
	};

	bool GMQueryCapability(GMCapability);
}

//! 机器的大小端模式的枚举值。
/*!
表示机器是大端模式或者是小端模式的枚举。
*/
enum class GMEndiannessMode
{
	// Never returns:
	Unknown = -1,  //!< 未知的模式，永远都不可能被返回

	// Modes:
	LittleEndian = 0, //!< 小端模式，数据的高字节保存在内存的高地址
	BigEndian = 1,    //!< 大端模式，数据的高字节保存在内存的低地址
};

/*! \def GM
    \brief 表示当前GameMachine运行实例。

    表示当前GameMachine运行实例。任何时候都推荐用GM来获取GameMachine实例。由于GameMachine实例为单例，因此
    不要尝试创建一个新的GameMachine实例。
*/
#define GM gm::GameMachine::instance()

class GMCanvas;

//! 程序运行时的各种状态。
/*!
  包含窗口、帧耗时等状态。状态将在每一帧结束之前更新。
  \sa GameMachine::getGameMachineRunningStates()
*/
struct GMGameMachineRunningStates
{
	// 每一帧更新的内容
	GMRect renderRect; //!< 当前窗口渲染窗口位置信息。
	GMfloat lastFrameElpased = 0; //!< 上一帧渲染锁花费的时间，单位是秒。
	GMfloat elapsedTime = 0; //!< 程序运行到现在为止的时间，单位是秒。
	GMfloat fps = 0; //!< 程序当前的帧率。
	bool crashDown = false; //!< 程序是否已崩溃。当遇到不可恢复的错误时，此项为true。

	// 由GameMachine生成
	GMRenderEnvironment renderEnvironment = GMRenderEnvironment::Invalid; //!< GameMachine采取的渲染环境。渲染环境决定着本程序将用何种数据结构、坐标系来进行运算。需要与窗口环境一致。

	// 下面字段由图形引擎生成
	GMfloat viewportTopLeftX = 0; //!< 视口左上角X坐标。
	GMfloat viewportTopLeftY = 0; //!< 视口左上角Y坐标。
	GMfloat minDepth = 0; //!< 近平面的深度值。
	GMfloat maxDepth = 1; //!< 远平面的深度值。
	GMString workingAdapterDesc; //!< 适配器信息。
	GMint sampleCount; //!< 多重采样数量。
	GMint sampleQuality; //!< 多重采样质量。
	bool vsyncEnabled = false; //!< 是否垂直同步。

	// 以下为常量
	// 永远不要更改以下2个值，它采用DirectX标准透视矩阵
	GMfloat farZ = 1.f; //!< 远平面的Z坐标。
	GMfloat nearZ = 0; //!< 近平面的Z坐标。

	GMEndiannessMode endiannessMode = GMEndiannessMode::Unknown;
};

GM_PRIVATE_OBJECT(GameMachine)
{
	GMClock clock;

	IWindow* mainWindow = nullptr;
	IFactory* factory = nullptr;
	IGraphicEngine* engine = nullptr;
	GMGlyphManager* glyphManager = nullptr;
	GMGamePackage* gamePackageManager = nullptr;
	GMConfigs* statesManager = nullptr;
	IGameHandler* gameHandler = nullptr;

	GMMessage lastMessage;

	// 内置调试窗口，他们本质是同一个对象，所以不能重复释放
	IWindow* consoleWindow = nullptr; 
	IDebugOutput* consoleOutput = nullptr;

	GMScopePtr<GMCamera> camera;
	Queue<GMMessage> messageQueue;
	Vector<IVirtualFunctionObject*> managerQueue;
	Vector<GMCanvas*> canvases;

	GMGameMachineRunningStates states;
	GMConfigs configs;
};

//! GameMachine类负责掌管整个进程的生命周期。
/*!
  GameMachine掌控整个进程的生命周期，提供对绘制、IO、内存和资源的管理。<BR>
  不要试图构造GameMachine，因为它是一个单例。请用GM宏来获取GameMachine单例。
*/
class GameMachine : public GMSingleton<GameMachine>
{
	DECLARE_PRIVATE(GameMachine)
	DECLARE_SINGLETON(GameMachine)

	enum
	{
		MAX_KEY_STATE_BITS = 512,
	};

protected:
	//! GameMachine构造函数
	/*!
	  构造一个GameMachine实例。不要试图自己创建一个GameMachine实例，而是使用GM宏来获取它的单例。
	*/
	GameMachine();

public:
	//! 初始化GameMachine。
	/*!
	  开发者应该在程序运行的最开始就初始化GameMachine，给GameMachine赋予绘制环境下的窗体、控制台处理器、
	当前环境下的工厂类，以及程序流程处理器。
	  \param mainWindow 程序运行的主窗口。可以使用gmui::GMUIFactory::createMainWindow创建。此对象生命周期由GameMachine管理。
	  \param consoleHandle 控制台处理器。当有日志或调试信息来的时候，将会调用到这个控制台处理器。可以使用gmui::GMUIFactory::createConsoleWindow创建。此对象生命周期由GameMachine管理。
	  \param factory 当前环境下的工厂类，用于创建纹理、字体管理器等绘制相关的对象。如果是在OpenGL下，可以直接创建GMGLFactory对象。此对象生命周期由GameMachine管理。
	  \param gameHandler 程序流程管理器，处理程序运行时的各个流程。此对象生命周期由GameMachine管理。
	  \param renderEnv 运行时的渲染环境。可以选择用OpenGL或DirectX11来进行渲染。此后的版本，也可能会增加更多的渲染环境。渲染环境一旦确立，将会影响工厂类返回的环境相关的实例。
	*/
	void init(
		AUTORELEASE IWindow* mainWindow,
		const GMConsoleHandle& consoleHandle,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler,
		GMRenderEnvironment renderEnv = GMRenderEnvironment::OpenGL
	);

	//! 获取绘制引擎。
	/*!
	  绘制引擎由初始化时的工厂类创建，提供最基础的绘制行为。
	  \return 程序绘制引擎。
	*/
	inline IGraphicEngine* getGraphicEngine() { D(d); return d->engine; }

	//! 获取程序主窗口。
	/*!
	  获取程序绘制的主窗口。
	  \return 程序主窗口。
	*/
	inline IWindow* getMainWindow() { D(d); return d->mainWindow; }

	//! 获取初始化时的工厂类。
	/*!
	  获取程序在初始化时存入的工厂类。工厂类将用于实例化各种引擎相关类型。
	  \return 程序工厂类。
	*/
	inline IFactory* getFactory() { D(d); return d->factory; }

	//! 获取字形管理器。
	/*!
	  获取程序的字形管理器，可以通过字体管理器拿出字符的字形位图。
	  \return 程序字形管理器。
	*/
	GMGlyphManager* getGlyphManager() { D(d); return d->glyphManager; }

	//! 获取资源管理器。
	/*!
	  所有程序的资产，如音乐、贴图、字体的原始数据，都可以从资源管理器中获取。
	  \return 程序资源管理器。
	*/
	GMGamePackage* getGamePackageManager() { D(d); return d->gamePackageManager; }

	//! 获取程序主相机。
	/*!
	  主相机用于主界面的绘制。
	  \return 程序主相机。
	*/
	GMCamera& getCamera() { D(d); return *d->camera; }

	//! 获取程序当前的运行时状态。
	/*!
	  如当前窗口大小、上一帧执行时间等。
	  \return 程序当前运行状态。
	*/
	inline const GMGameMachineRunningStates& getGameMachineRunningStates() const { D(d); return d->states; }

	//! 获取程序当前的配置。
	/*!
	  通过设置程序配置，可以激活运行时的一些行为，如绘制调试信息、输出性能表等。程序所有的配置存在此对象中。用户可以获取此对象并且修改它。
	  \return 程序当前配置。
	*/
	inline GMConfigs& getConfigs() { D(d); return d->configs; }

	//! 初始化一个对象数据代理。
	/*!
	  初始化一个对象数据代理。每当新建一个GMModel时，需要调用此方法，为GMModel设置一个数据代理。此方法将用工厂类来为GMModel创建一个GMModelDataProxy对象。<br>
	  在创建之后，数据代理马上会将GMModel顶点数据传输到GPU。
	  \param model 需要创建数据代理的模型对象。创建好的GMModelDataProxy会绑定在此对象上。在此对象析构时，GMModelDataProxy也会析构，用户不需要关心它的生命周期。
	*/
	void createModelDataProxyAndTransfer(GMModel* model);

	//! 获取当前机器的大小端模式。
	/*!
	  此方法会将返回值保存起来，下一次调用的时候，直接返回其保存值。
	  \return 当前机器大小端模式。
	*/
	GMEndiannessMode getMachineEndianness();

	//! 发送一条GameMachine的消息。
	/*!
	  发送一条GameMachine的消息。发送消息之后，此消息将会在下一轮消息循环时被执行。
	  \param msg 需要发送的GameMachine消息。
	  \sa startGameMachine()
	*/
	void postMessage(GMMessage msg);

	//! 获取最后一条GameMachine消息。
	/*!
	  获取最后一条GameMachine消息。此方法仅仅是获取GameMachine消息队列中的最后一条消息，并不会移除此消息。如果
	在某些对象中，需要处理GameMachine的消息，如当窗口大小改变时处理对象中的一些行为，可以调用此方法获取消息。
	  \return GameMachine消息。
	*/
	GMMessage peekMessage();

	//! 开始运行GameMachine。
	/*!
	  当GameMachine实例初始化之后，调用此方法开始程序内的游戏循环。<BR>
	  在游戏循环之前，它首先会新建主窗口和调试窗口，创建内部各种管理器并初始化它们。接下来，触发游戏时钟，程序运行时
	间从此刻开始计算。在此之后，程序的消息循环正式开始。循环会先处理来自系统的消息，如窗口、鼠标、键盘消息，接下来处
	理GameMachine消息，然后调用游戏流程管理器管理游戏流程，最后再更新此帧的状态。
	  \sa init()
	*/
	void startGameMachine();

	//! 设置当前的运行状态。
	/*!
	  用户永远不要调用这个方法来修改运行状态。这个方法将在窗口初始化或者其他特殊场合下被调用，并在每一帧更新。<BR>
	  一般，只需要调用getGameMachineRunningStates获取状态即可。
	  \param states 需要更新的运行状态
	  \sa getGameMachineRunningStates()
	*/
	void setGameMachineRunningStates(const GMGameMachineRunningStates& states)
	{
		D(d);
		d->states = states;
	}

	//! 注册一个画布到系统消息循环。
	/*!
	  GameMachine不会管理被注册的画布的生命周期，如果画布被析构，一定要记得在合适的时机释放画布。
	  \param canvas 待注册的画布。
	*/
	void registerCanvas(GMCanvas* canvas);

	//! 触发所有被注册的画布的消息处理事件。
	/*!
	  通过此方法，使得被注册的画布有了处理系统消息的时机。一般不需要手动调用此方法，因为GMUIGameMachineWindowBase会调用它。
	  \return 是否已经处理了此消息。如果返回true，则此消息不会再继续传递。
	*/
	bool dispatchMessageToCanvases(GMuint uMsg, GMWParam wParam, GMLParam lParam, GMLResult* lRes);


private:
	void runEventLoop();
	bool renderFrame();
	template <typename T, typename U> void registerManager(T* newObject, OUT U** manager);
	void terminate();
	bool handleMessages();
	bool handleMessage(const GMMessage& msg);
	void updateGameMachineRunningStates();
	void setRenderEnvironment(GMRenderEnvironment renv);
	bool checkCrashDown();
	void handlerEvents();
	void updateManagers();
	void translateSystemEvent(GMuint uMsg, GMWParam wParam, GMLParam lParam, GMLResult* lRes, OUT GMSystemEvent** event);
};

END_NS
#endif
