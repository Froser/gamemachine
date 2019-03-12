#include "stdafx.h"
#include "gamemachinewidget.h"
#include <gamemachine.h>

namespace
{
	using namespace gm;

	bool g_isGameMachineInited = false;

	IWindow* createGameMachineChildWindow(IGameHandler* handler, float renderWidth, float renderHeight, HWND hContainer, IFactory* factory)
	{
		IWindow* window = NULL;
		factory->createWindow(NULL, 0, &window);
		GM_ASSERT(window);
		window->setHandler(handler);

		GMWindowDesc wndAttrs;
		wndAttrs.createNewWindow = false;
		wndAttrs.existWindowHandle = hContainer;
		wndAttrs.rc = { 0, 0, renderWidth, renderHeight };
		window->create(wndAttrs);
		return window;
	}

	GMAtomic<GMsize_t>& getGameMachineRefCount()
	{
		static GMAtomic<GMsize_t> s_gmrefcnt;
		return s_gmrefcnt;
	}
}

namespace gm
{
	GameMachineWidget::~GameMachineWidget()
	{
		if (m_gamemachineSet)
		{
			if (!--getGameMachineRefCount())
				GM.finalize();
		}
	}

	void GameMachineWidget::setGameMachine(const GMGameMachineDesc& desc, float renderWidth, float renderHeight, IGameHandler* handler)
	{
		if (!g_isGameMachineInited)
		{
			GMGameMachineDesc tmpDesc = desc;
			tmpDesc.runningMode = GMGameMachineRunningMode::ApplicationMode;
			GM.init(desc);
			GM.startGameMachineWithoutMessageLoop();
			g_isGameMachineInited = true;
		}

		IWindow* gamemachineWindow = createGameMachineChildWindow(handler, renderWidth, renderHeight, (HWND)winId(), desc.factory);
		GM.addWindow(gamemachineWindow);
		setRenderContext(gamemachineWindow->getContext());
		setAttribute(Qt::WA_PaintOnScreen, true);
		setAttribute(Qt::WA_NativeWindow, true);
		GM.handleMessages();

		m_gamemachineSet = true;
		++getGameMachineRefCount();
	}

	void GameMachineWidget::setRenderContext(const IRenderContext* context)
	{
		m_context = context;
	}

	bool GameMachineWidget::event(QEvent* e)
	{
		// 是否需要在所有Event时都来handle messages?
		if (g_isGameMachineInited)
			GM.handleMessages();

		return QWidget::event(e);
	}

	QPaintEngine* GameMachineWidget::paintEngine() const
	{
		// 返回nullptr，因为我们不需要QT替我们绘制。
		return nullptr;
	}

	void GameMachineWidget::paintEvent(QPaintEvent *event)
	{
		if (m_context)
			GM.renderFrame(m_context->getWindow());
	}
}
