#include "stdafx.h"
#include "gamemachinewidget.h"
#include <gamemachine.h>

namespace
{
	bool g_isGameMachineInited = false;

	IWindow* createGameMachineChildWindow(IGameHandler* handler, HWND hContainer, IFactory* factory)
	{
		IWindow* window = NULL;
		factory->createWindow(NULL, 0, &window);
		GM_ASSERT(window);

		window->setHandler(handler);

		GMWindowDesc wndAttrs;
		wndAttrs.createNewWindow = false;
		wndAttrs.existWindowHandle = hContainer;
		window->create(wndAttrs);
		return window;
	}
}


namespace shell
{
	GameMachineWidget::GameMachineWidget(QWidget* parent /*= nullptr*/)
		: QWidget(parent)
	{

	}

	GameMachineWidget* GameMachineWidget::createGameMachineWidget(const GMGameMachineDesc& desc, IGameHandler* handler, QWidget* parent/* = nullptr*/)
	{
		if (!g_isGameMachineInited)
		{
			GMGameMachineDesc tmpDesc = desc;
			tmpDesc.runningMode = GMGameMachineRunningMode::ApplicationMode;
			GM.init(desc);
			GM.startGameMachineWithoutMessageLoop();
			g_isGameMachineInited = true;
		}

		GameMachineWidget* gmwidget = new GameMachineWidget(parent);
		IWindow* gamemachineWindow = createGameMachineChildWindow(handler, (HWND)gmwidget->winId(), desc.factory);
		GM.addWindow(gamemachineWindow);
		gmwidget->setRenderContext(gamemachineWindow->getContext());
		gmwidget->setAttribute(Qt::WA_PaintOnScreen, true);
		gmwidget->setAttribute(Qt::WA_NativeWindow, true);
		return gmwidget;
	}

	void GameMachineWidget::setRenderContext(const IRenderContext* context)
	{
		m_context = context;
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
