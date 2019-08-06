#include "stdafx.h"
#include "handler.h"
#include "procedures.h"
#include <gmshaderhelper.h>

namespace
{
	Handler* s_handler;
}

Handler::Handler(IWindow* window, Config&& c) GM_NOEXCEPT
	: m_mainWindow(window)
	, m_world(nullptr)
	, m_config(std::move(c))
{
	s_handler = this;
}

Handler::~Handler()
{
	s_handler = nullptr;
}

GMGameWorld* Handler::getWorld()
{
	return m_world.get();
}

IWindow* Handler::getWindow()
{
	return m_mainWindow;
}

GMFontHandle Handler::getFontHandleByName(const GMString& name)
{
	return m_config.fontHandles[name];
}

GMfloat Handler::getTick()
{
	return m_clock.elapsedFromStart();
}

Handler* Handler::instance()
{
	return s_handler;
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

	{
		GMBuffer buf;
		GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
		pk->readFile(GMPackageIndex::Fonts, m_config.fontCN, &buf);
		GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
		glyphManager->setDefaultFontCN(font);
	}

	{
		GMBuffer buf;
		GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
		pk->readFile(GMPackageIndex::Fonts, m_config.fontEN, &buf);
		GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
		glyphManager->setDefaultFontEN(font);
	}

	for (const auto& fontPair : m_config.fonts)
	{
		GMBuffer buf;
		GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
		pk->readFile(GMPackageIndex::Fonts, fontPair.second, &buf);
		GMFontHandle fontHandle = glyphManager->addFontByMemory(std::move(buf));
		m_config.fontHandles[fontPair.first] = fontHandle;
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
		m_procedures->run(GM.getRunningStates().lastFrameElapsed);
		break;
	}
	case GameMachineHandlerEvent::Update:
	{
		GMfloat dt = GM.getRunningStates().lastFrameElapsed;
		m_procedures->update(dt);
		break;
	}
	case GameMachineHandlerEvent::Terminate:
		m_procedures->finalize();
		break;
	}
}
