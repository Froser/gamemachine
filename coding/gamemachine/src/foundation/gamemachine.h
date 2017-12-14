#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include <gmcommon.h>
#include <utilities.h>
#include <input.h>
#include <gmassets.h>
#include "../gmdata/glyph/gmglyphmanager.h"
#include "../gmdata/gamepackage/gmgamepackage.h"
#include "../gmengine/gmcamera.h"
#include "gmthreads.h"

BEGIN_NS

#define GM gm::GameMachine::instance()

class GMCursorGameObject;
struct GMGameMachineRunningStates
{
	GMRect clientRect;
	GMRect windowRect;
	GMfloat lastFrameElpased = 0;
	bool crashDown = false;
};

GM_PRIVATE_OBJECT(GameMachine)
{
	GMClock clock;

	IWindow* mainWindow = nullptr;
	IFactory* factory = nullptr;
	IGraphicEngine* engine = nullptr;
	GMGlyphManager* glyphManager = nullptr;
	GMGamePackage* gamePackageManager = nullptr;
	GMStates* statesManager = nullptr;
	IGameHandler* gameHandler = nullptr;

	GameMachineMessage lastMessage;

	// 内置调试窗口，他们本质是同一个对象，所以不能重复释放
	IWindow* consoleWindow = nullptr; 
	IDebugOutput* consoleOutput = nullptr;

	GMCamera camera;
	Queue<GameMachineMessage> messageQueue;
	Vector<IDispose*> manangerQueue;

	GMGameMachineRunningStates states;
	GMCursorGameObject* cursor;
};

//! GameMachine类负责掌管整个进程的生命周期。
/*! GameMachine掌控整个进程的生命周期，提供对绘制、IO、内存和资源
    的管理。不要试图构造GameMachine，因为它是一个单例。
    请用GM宏来获取GameMachine单例。
*/
class GameMachine : public GMSingleton<GameMachine>
{
	friend class GMSingleton<GameMachine>;

	DECLARE_PRIVATE(GameMachine)
	DECLARE_SINGLETON(GameMachine)

	enum
	{
		MAX_KEY_STATE_BITS = 512,
	};

public:
	//! 机器的大小端模式的枚举值。
	/*! 表示机器是大端模式或者是小端模式的枚举。*/
	enum EndiannessMode
	{
		// Never returns:
		UNKNOWN_YET = -1,  /*!< 未知的模式，永远都不可能被返回 */

		// Modes:
		LITTLE_ENDIAN = 0, /*!< 小端模式，数据的高字节保存在内存的高地址 */
		BIG_ENDIAN = 1,    /*!< 大端模式，数据的高字节保存在内存的低地址 */
	};

protected:
	//! GameMachine构造函数
	/*!
	  构造一个GameMachine实例。不要试图自己创建一个GameMachine实例，而是使用GM宏来获取它的单例。
	*/
	GameMachine() = default;

public:
	//! 初始化GameMachine。
	/*!
	  开发者应该在程序运行的最开始就初始化GameMachine，给GameMachine赋予绘制环境下的窗体、控制台处理器、
	  当前环境下的工厂类，以及程序流程处理器。
	  \params mainWindow 程序运行的主窗口。可以使用gmui::GMUIFactory::createMainWindow创建。此对象生命周期由GameMachine管理。
	  \params consoleHandle 控制台处理器。当有日志或调试信息来的时候，将会调用到这个控制台处理器。可以使用gmui::GMUIFactory::createConsoleWindow创建。此对象生命周期由GameMachine管理。
	  \params factory 当前环境下的工厂类，用于创建纹理、字体管理器等绘制相关的对象。如果是在OpenGL下，可以直接创建GMGLFactory对象。此对象生命周期由GameMachine管理。
	  \params gameHandler 程序流程管理器，处理程序运行时的各个流程。此对象生命周期由GameMachine管理。
	*/
	void init(
		AUTORELEASE IWindow* mainWindow,
		const GMConsoleHandle& consoleHandle,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler
	);

	//! 获取绘制引擎。
	/*!
	  获取当前的绘制引擎。绘制引擎由初始化时的工厂类创建，提供最基础的绘制行为。
	*/
	inline IGraphicEngine* getGraphicEngine() { D(d); return d->engine; }

	//! 获取程序主窗口。
	/*!
	  获取程序绘制的主窗口。
	*/
	inline IWindow* getMainWindow() { D(d); return d->mainWindow; }

	//! 获取初始化时的工厂类。
	/*!
	  获取程序在初始化时存入的工厂类。
	*/
	inline IFactory* getFactory() { D(d); return d->factory; }

	//! 获取当前的程序配置。
	/*!
	  获取当前的程序配置。通过设置程序配置，可以激活运行时的一些行为，如绘制调试信息、输出性能表等。
	*/
	GMStates* getStatesManager() { D(d); return d->statesManager; }

	//! 获取字体管理器。
	/*!
	  获取程序的字体管理器，可以通过字体管理器拿出字符的字形位图。
	*/
	GMGlyphManager* getGlyphManager() { D(d); return d->glyphManager; }

	// 资源管理
	GMGamePackage* getGamePackageManager() { D(d); return d->gamePackageManager; }

	// 相机管理
	GMCamera& getCamera() { D(d); return d->camera; }

	// 时间管理
	inline GMfloat getFPS() { D(d); return d->clock.getFps(); }
	inline GMfloat getGameTimeSeconds() { D(d); return d->clock.getTime(); }

	// 状态管理
	inline const GMGameMachineRunningStates& getGameMachineRunningStates() const { D(d); return d->states; }

	// 绘制管理
	void initObjectPainter(GMModel* model);

	// 大小端模式
	EndiannessMode getMachineEndianness();

	// 发送事件
	void postMessage(GameMachineMessage msg);
	GameMachineMessage peekMessage();

	inline void setCursor(GMCursorGameObject* cursor) { D(d); d->cursor = cursor; }
	inline GMCursorGameObject* getCursor() { D(d); return d->cursor; }

	void startGameMachine();

private:
	template <typename T, typename U> void registerManager(T* newObject, OUT U** manager);
	void terminate();
	bool handleMessages();
	void initInner();
	void updateGameMachineRunningStates();
};

END_NS
#endif
