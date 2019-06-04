#include "stdafx.h"
#include "handler.h"
#include "procedures.h"
#include <gmshaderhelper.h>

Handler::Handler(IWindow* window)
	: m_mainWindow(window)
	, m_world(nullptr)
{
}

Handler::~Handler()
{
}

GMGameWorld* Handler::getWorld()
{
	return m_world.get();
}

IWindow* Handler::getWindow()
{
	return m_mainWindow;
}

GMfloat Handler::getTick()
{
	return m_clock.elapsedFromStart();
}

void Handler::onLoadShaders(const IRenderContext* context)
{
	// 使用预设的着色器程序
	GMShaderHelper::loadExtensionShaders(context);
	GMShaderHelper::loadShader(context);
}

void Handler::init(const IRenderContext* context)
{
	GMGamePackage* pk = GM.getGamePackageManager();

#if GM_WINDOWS
#	ifdef GM_DEBUG
	pk->loadPackage("D:/gmpk");
#	else
	pk->loadPackage((GMPath::getCurrentPath() + L"gm.pk0"));
#	endif
#else
#	ifdef GM_DEBUG
	pk->loadPackage("/home/froser/Documents/gmpk");
#	else
	pk->loadPackage((GMPath::getCurrentPath() + L"gm.pk0"));
#	endif
#endif

	{
		GMBuffer buf;
		GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
		pk->readFile(GMPackageIndex::Fonts, L"simhei.ttf", &buf);
		GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
		glyphManager->setCN(font);
	}

	{
		GMBuffer buf;
		GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
		pk->readFile(GMPackageIndex::Fonts, L"times.ttf", &buf);
		GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
		glyphManager->setEN(font);
	}

	context->getEngine()->setShaderLoadCallback(this);
	m_world.reset(new GMGameWorld(context));
	m_procedures.reset(new Procedures(this));
}

void Handler::start()
{
	IInput* inputManager = m_mainWindow->getInputManager();
	inputManager->getIMState().activate(GMKL_United_States);
}

void Handler::updateTick()
{
	m_clock.update();
}

void Handler::event(GameMachineHandlerEvent evt)
{
	switch (evt)
	{
	case GameMachineHandlerEvent::Render:
	{
		IGraphicEngine* engine = getWorld()->getContext()->getEngine();
		engine->getDefaultFramebuffers()->clear();
		m_procedures->run(GM.getRunningStates().lastFrameElpased);
		break;
	}
	case GameMachineHandlerEvent::Terminate:
		m_procedures->finalize();
		break;
	}
}
