#include "stdafx.h"
#include "procedures.h"
#include "handler.h"
#include "helper.h"
#include <gmm.h>

Procedures::Procedures(Handler* handler)
	: m_handler(handler)
	, m_p(*this)
	, m_procedure(ShowLogo)
{
}

void Procedures::run(GMDuration dt)
{
	m_p.callMainThreadFunctions();
	switch (m_procedure)
	{
	case ShowLogo:
		m_p.showLogo(dt);
		break;
	case LoadingScene:
		m_p.loadingScene(dt);
		break;
	case Play:
		m_p.play(dt);
		break;
	}
}

void Procedures::finalize()
{
	m_p.finalize();
}

ProceduresPrivate::ProceduresPrivate(Procedures& p)
	: m_procedures(p)
	, m_title(nullptr)
	, m_assetsLoaded(false)
	, m_timeline(p.m_handler->getWindow()->getContext(), p.m_handler->getWorld())
	, m_timelineStarted(false)
{
	m_audioPlayer = gmm::GMMFactory::getAudioPlayer();
	m_audioReader = gmm::GMMFactory::getAudioReader();
}

void ProceduresPrivate::finalize()
{
	if (m_bgmSrc)
		m_bgmSrc->stop();

	for (auto& f : m_managedFutures)
	{
		if (f.valid())
			f.wait();
	}
}

void ProceduresPrivate::showLogo(GMDuration dt)
{
	static std::once_flag s_flag;
	std::call_once(s_flag, [this]() {
		async(GMAsync::async(GMAsync::Async, [this]() {

			GMBuffer buffer;
			GM.getGamePackageManager()->readFile(GMPackageIndex::Scripts, "timeline.xml", &buffer);
			GM_ASSERT(buffer.getSize() > 0);
			if (buffer.getSize() > 0)
			{
				buffer.convertToStringBuffer();
				m_timeline.parse(GMString(reinterpret_cast<char*>(buffer.getData())));
				invokeInMainThread([this]() {
					m_timeline.play();
					m_assetsLoaded = true;
				});
			}
		}));
	});

	if (!m_title)
	{
		IWindow* window = m_procedures.m_handler->getWindow();
		m_title = new GMTextGameObject(window->getRenderRect());
		m_title->setColorType(GMTextColorType::Plain);
		m_title->setFontSize(32);
		m_title->setColor(GMVec4(0, 0, 0, 1));
		m_title->setText("GameMachine Presents");
		GMRect rc = { 0, 0, 512, 32 };
		rc = Helper::getMiddleRectOfWindow(rc, window);
		rc.y -= 40;
		m_title->setGeometry(rc);

		m_titleAnimation.setTargetObjects(m_title);
		m_titleAnimation.addKeyFrame(new FontColorAnimationKeyframe(m_title, GMVec4(1, 1, 1, 1), 1));
		m_titleAnimation.addKeyFrame(new FontColorAnimationKeyframe(m_title, GMVec4(1, 1, 1, 1), 2));
		m_titleAnimation.addKeyFrame(new FontColorAnimationKeyframe(m_title, GMVec4(0, 0, 0, 0), 3));
		m_titleAnimation.reset();
		m_titleAnimation.play();

		m_procedures.m_handler->getWorld()->addObjectAndInit(m_title);
		m_procedures.m_handler->getWorld()->addToRenderList(m_title);
	}

	m_titleAnimation.update(dt);
	if (m_titleAnimation.isFinished())
	{
		IWindow* window = m_procedures.m_handler->getWindow();
		m_procedures.m_handler->getWorld()->removeObject(m_title);

		m_title = new GMTextGameObject(window->getRenderRect());
		m_title->setColorType(GMTextColorType::Plain);
		m_title->setFontSize(32);
		m_title->setColor(GMVec4(0, 0, 0, 1));
		m_title->setText("Producer: Froser");
		GMRect rc = { 0, 0, 400, 32 };
		rc = Helper::getMiddleRectOfWindow(rc, window);
		rc.y -= 40;
		m_title->setGeometry(rc);
		m_titleAnimation.clearFrames();
		m_titleAnimation.setTargetObjects(m_title);
		m_titleAnimation.addKeyFrame(new FontColorAnimationKeyframe(m_title, GMVec4(1, 1, 1, 1), 1));
		m_titleAnimation.reset();
		m_titleAnimation.play();

		m_procedures.m_handler->getWorld()->addObjectAndInit(m_title);
		m_procedures.m_handler->getWorld()->addToRenderList(m_title);

		static std::once_flag s_flag;
		std::call_once(s_flag, [this]() {
			async(GMAsync::async(GMAsync::Async, [this]() {
				GMBuffer bgBuffer;
				GMString fn;
				GM.getGamePackageManager()->readFile(GMPackageIndex::Audio, L"Thomas Greenberg - Curly Wurly.mp3", &bgBuffer, &fn);
				if (bgBuffer.getSize() > 0)
				{
					IAudioFile* musicFile;
					if (m_audioReader->load(bgBuffer, &musicFile))
					{
						IAudioSource* musicSrc;
						m_audioPlayer->createPlayerSource(musicFile, &musicSrc);
						m_bgmSrc.reset(musicSrc);
						invokeInMainThread([this]() {
							m_bgmSrc->play(false);
						});
					}
					else
					{
						gm_warning(gm_dbg_wrap("BGM load failed: {0}"), fn);
					}
				}
				else
				{
					gm_warning(gm_dbg_wrap("BGM not found: {0}"), fn);
				}
			}));
		});
		m_procedures.m_procedure = Procedures::LoadingScene;
	}

	m_procedures.m_handler->getWorld()->renderScene();
}

void ProceduresPrivate::loadingScene(GMDuration dt)
{
	// 如果资源没有读取完成，将会卡在这里
	m_titleAnimation.update(dt);
	m_procedures.m_handler->getWorld()->renderScene();

	if (m_assetsLoaded)
	{
		m_procedures.m_procedure = Procedures::Play;
	}
}

void ProceduresPrivate::play(GMDuration dt)
{
	if (m_titleAnimation.isFinished() && !m_timelineStarted)
	{
		m_titleAnimation.clearFrames();
		m_titleAnimation.addKeyFrame(new FontColorAnimationKeyframe(m_title, GMVec4(0, 0, 0, 0), 1));
		m_titleAnimation.reset();
		m_titleAnimation.play();
		m_timelineStarted = true;
	}

	// 按照脚本播放动画
	m_timeline.update(dt);

	m_titleAnimation.update(dt);
	m_procedures.m_handler->getWorld()->renderScene();
}

void ProceduresPrivate::invokeInMainThread(std::function<void()> f)
{
	m_funcQueue.push(f);
}

void ProceduresPrivate::callMainThreadFunctions()
{
	if (!m_funcQueue.empty())
	{
		std::function<void()> func = m_funcQueue.top();
		m_funcQueue.pop();
		func();
	}
}

void ProceduresPrivate::async(GMFuture<void>&& f)
{
	m_managedFutures.push_back(std::move(f));
}
