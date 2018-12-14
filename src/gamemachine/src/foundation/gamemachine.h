#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <gminput.h>
#include <gmassets.h>
#include <gmglyphmanager.h>
#include <gmgamepackage.h>
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

enum class GMProcessorArchitecture
{
	AMD64,
	ARM,
	IA64,
	Intel,
	Unknown,
};

/*! \def GM
    \brief 表示当前GameMachine运行实例。

    表示当前GameMachine运行实例。任何时候都推荐用GM来获取GameMachine实例。由于GameMachine实例为单例，因此
    不要尝试创建一个新的GameMachine实例。
*/
#define GM gm::GameMachine::instance()

class GMWidget;

struct GMSystemInfo
{
	GMEndiannessMode endiannessMode = GMEndiannessMode::Unknown;
	GMint32 numberOfProcessors = 0;
	GMProcessorArchitecture processorArchitecture;
};

//! 程序运行时的各种状态。
/*!
  包含窗口、帧耗时等状态。状态将在每一帧结束之前更新。
  \sa GameMachine::getGameMachineRunningStates()
*/
struct GMGameMachineRunningStates
{
	GMSystemInfo systemInfo;

	// 每一帧更新的内容
	GMDuration lastFrameElpased = 0; //!< 上一帧渲染锁花费的时间，单位是秒。
	GMDuration elapsedTime = 0; //!< 程序运行到现在为止的时间，单位是秒。
	GMfloat fps = 0; //!< 程序当前的帧率。
	bool crashDown = false; //!< 程序是否已崩溃。当遇到不可恢复的错误时，此项为true。

	// 由GameMachine生成
	GMRenderEnvironment renderEnvironment = GMRenderEnvironment::Invalid; //!< GameMachine采取的渲染环境。渲染环境决定着本程序将用何种数据结构、坐标系来进行运算。需要与窗口环境一致。

	// 以下为常量
	// 永远不要更改以下2个值，它采用DirectX标准透视矩阵
	GMfloat farZ = 1.f; //!< 远平面的Z坐标。
	GMfloat nearZ = 0; //!< 近平面的Z坐标。
};

//! 描述GameMachine运行时的模式。
enum class GMGameMachineRunningMode
{
	GameMode, //!< 游戏模式。游戏模式下，GameMachine会按照固定帧率刷新画面。
	ApplicationMode, //!< 应用模式。应用模式下，需要手动调用GameMachine更新方法才可以更新一帧画面。
};

//! 描述一个GameMachine的执行环境。
struct GMGameMachineDesc
{
	AUTORELEASE IFactory* factory = nullptr; //!< 当前环境下的工厂类，用于创建纹理、字体管理器等绘制相关的对象。如果是在OpenGL下，可以直接创建GMGLFactory对象。此对象生命周期由GameMachine管理。
	GMRenderEnvironment renderEnvironment = GMRenderEnvironment::OpenGL; //!< 运行时的渲染环境。可以选择用OpenGL或DirectX11来进行渲染。此后的版本，也可能会增加更多的渲染环境。渲染环境一旦确立，将会影响工厂类返回的环境相关的实例。
	GMGameMachineRunningMode runningMode = GMGameMachineRunningMode::GameMode;
};

GM_PRIVATE_OBJECT_UNALIGNED(GameMachine)
{
	GMClock clock;
	bool inited = false;
	Set<IWindow*> windows;
	IFactory* factory = nullptr;
	GMGamePackage* gamePackageManager = nullptr;
	GMConfigs* statesManager = nullptr;
	GMMessage lastMessage;
	Queue<GMMessage> messageQueue;
	Vector<IVirtualFunctionObject*> managerQueue;
	GMGameMachineRunningStates states;
	GMGameMachineRunningMode runningMode;
	GMConfigs configs;
};

//! GameMachine类负责掌管整个进程的生命周期。
/*!
  GameMachine掌控整个进程的生命周期，提供对绘制、IO、内存和资源的管理。<BR>
  不要试图构造GameMachine，因为它是一个单例。请用GM宏来获取GameMachine单例。
*/
class GM_EXPORT GameMachine
{
	GM_DECLARE_PRIVATE_NGO(GameMachine)

public:
	static GameMachine& instance();

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
	~GameMachine() = default;

public:
	//! 初始化GameMachine。
	/*!
	  开发者应该在程序添加窗体后就初始化GameMachine，给GameMachine赋予绘制环境下的窗体、控制台处理器、当前环境下的工厂类，以及程序流程处理器。
	  \param desc GameMachine的描述信息。
	*/
	void init(const GMGameMachineDesc& desc = GMGameMachineDesc());

	//! 获取初始化时的工厂类。
	/*!
	  获取程序在初始化时存入的工厂类。工厂类将用于实例化各种引擎相关类型。
	  \return 程序工厂类。
	*/
	inline IFactory* getFactory() { D(d); return d->factory; }

	//! 获取资源管理器。
	/*!
	  所有程序的资产，如音乐、贴图、字体的原始数据，都可以从资源管理器中获取。
	  \return 程序资源管理器。
	*/
	inline GMGamePackage* getGamePackageManager() { D(d); return d->gamePackageManager; }

	//! 获取程序当前的运行时状态。
	/*!
	  如当前窗口大小、上一帧执行时间等。
	  \return 程序当前运行状态。
	*/
	GM_META_METHOD inline const GMGameMachineRunningStates& getRunningStates() const { D(d); return d->states; }

	//! 获取程序当前的配置。
	/*!
	  通过设置程序配置，可以激活运行时的一些行为，如绘制调试信息、输出性能表等。程序所有的配置存在此对象中。用户可以获取此对象并且修改它。
	  \return 程序当前配置。
	*/
	inline GMConfigs& getConfigs() { D(d); return d->configs; }

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

	//! 将系统事件翻译成GameMachine事件。
	/*!
	  翻译后的事件将是平台无关的。
	*/
	void translateSystemEvent(GMuint32 uMsg, GMWParam wParam, GMLParam lParam, OUT GMSystemEvent** event);

	//! 为GameMachine注册个窗口。
	/*!
	  添加到GameMachine后的窗口将响应程序的消息循环和各种事件。
	  \param window 待增加的窗口。
	*/
	void addWindow(AUTORELEASE IWindow* window);

	//! 为GameMachine移除一个窗口。
	/*!
	  移除后的窗口不再响应任何事件。
	*/
	void removeWindow(IWindow* window);

	//! 渲染一帧画面。
	/*!
		在不同的描述设置下，GameMachine有不一样的行为：<BR>
		游戏模式: 游戏模式下，GameMachine会按照固定帧率刷新画面。不得在这种模式下手动调用此方法。<BR>
		应用模式: 应用模式下，需要手动调用此方法才可以更新一帧画面。
		\param window 需要渲染的window。如果为空指针，则更新每一个注册的window。
		\sa GameMachineDesc
	*/
	bool renderFrame(IWindow* window = nullptr);

	//! 退出程序。
	GM_META_METHOD void exit();

private:
	void initSystemInfo();
	void runEventLoop();
	template <typename T, typename U> void registerManager(T* newObject, OUT U** manager);
	void terminate();
	bool handleMessages();
	bool handleMessage(const GMMessage& msg);
	void updateGameMachineRunningStates();
	void setRenderEnvironment(GMRenderEnvironment renv);
	bool checkCrashDown();
	void beginHandlerEvents(IWindow* window);
	void endHandlerEvents(IWindow* window);
	void eachHandler(std::function<void(IWindow*, IGameHandler*)> action);
};

END_NS
#endif
