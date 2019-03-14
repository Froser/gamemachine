#include "stdafx.h"
#include "handler.h"

#include <gmgl.h>
#include <gmglhelper.h>
#if GM_USE_DX11
#include <gmdx11.h>
#include <gmdx11helper.h>
#endif
#include "..\src\gmphysics\gmdiscretedynamicsworld.h"

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
			glyphManager->setCN(font);
		}

		{
			gm::GMBuffer buf;
			gm::GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
			pk->readFile(gm::GMPackageIndex::Fonts, L"times.ttf", &buf);
			gm::GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
			glyphManager->setEN(font);
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
		auto& env = GM.getRunningStates().renderEnvironment;
		if (env == gm::GMRenderEnvironment::OpenGL)
		{
			gm::GMGLHelper::loadShader(
				context,
				L"gl/main.vert",
				L"gl/main.frag",
				L"gl/deferred/geometry_pass_main.vert",
				L"gl/deferred/geometry_pass_main.frag",
				L"gl/deferred/light_pass_main.vert",
				L"gl/deferred/light_pass_main.frag",
				L"gl/filters/filters.vert",
				L"gl/filters/filters.frag"
			);
		}
		else
		{
#if GM_USE_DX11
			gm::GMDx11Helper::loadShader(context, L"dx11/effect.fx");
#else
			GM_ASSERT(false);
#endif
		}
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