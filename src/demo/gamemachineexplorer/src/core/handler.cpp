#include "stdafx.h"
#include "handler.h"

#include <gmgl.h>
#include <gmglhelper.h>
#include "..\src\gmphysics\gmdiscretedynamicsworld.h"
#include <gmshaderhelper.h>

namespace core
{
	Handler::~Handler()
	{
		GM_delete(m_world);
	}

	void Handler::init(const IRenderContext* context)
	{
		m_context = context;

		gm::GMGamePackage* pk = GM.getGamePackageManager();

#if GM_WINDOWS
#	ifdef GM_DEBUG
		pk->loadPackage("D:/gmpk");
#	else
		pk->loadPackage((gm::GMPath::getCurrentPath() + L"gm.pk0"));
#	endif
#else
#	ifdef GM_DEBUG
		pk->loadPackage("/home/froser/Documents/gmpk");
#	else
		pk->loadPackage((gm::GMPath::getCurrentPath() + L"gm.pk0"));
#	endif
#endif

		{
			gm::GMBuffer buf;
			gm::GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
			pk->readFile(gm::GMPackageIndex::Fonts, L"simhei.ttf", &buf);
			gm::GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
			glyphManager->setDefaultFontCN(font);
		}

		{
			gm::GMBuffer buf;
			gm::GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
			pk->readFile(gm::GMPackageIndex::Fonts, L"times.ttf", &buf);
			gm::GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
			glyphManager->setDefaultFontEN(font);
		}

		context->getEngine()->setShaderLoadCallback(this);
		m_world = new GMGameWorld(context);
		m_physicsWorld = new GMDiscreteDynamicsWorld(m_world);
	}

	void Handler::start()
	{
		GM.renderFrame(m_context->getWindow());
	}

	void Handler::onLoadShaders(const IRenderContext* context)
	{
		gm::GMShaderHelper::loadExtensionShaders(context);
		gm::GMShaderHelper::loadShader(context);
	}

	void Handler::event(GameMachineHandlerEvent evt)
	{
		switch (evt)
		{
		case GameMachineHandlerEvent::Render:
		{
			m_context->getEngine()->getDefaultFramebuffers()->clear();
			m_world->renderScene();
			break;
		}
		}
	}

}