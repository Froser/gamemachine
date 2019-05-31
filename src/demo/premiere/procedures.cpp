#include "stdafx.h"
#include "procedures.h"
#include "handler.h"
#include "helper.h"

Procedures::Procedures(Handler* handler)
	: m_handler(handler)
	, m_p(*this)
	, m_procedure(ShowLogo)
{
}

void Procedures::run(GMDuration dt)
{
	switch (m_procedure)
	{
	case ShowLogo:
		m_p.showLogo(dt);
		break;
	}
}

ProceduresPrivate::ProceduresPrivate(Procedures& p)
	: m_procedures(p)
	, m_title(nullptr)
{
}

void ProceduresPrivate::showLogo(GMDuration dt)
{
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
		m_titleAnimation.addKeyFrame(new FontColorAnimationKeyframe(m_title, GMVec4(1, 1, 1, 1), 2));
		m_titleAnimation.addKeyFrame(new FontColorAnimationKeyframe(m_title, GMVec4(1, 1, 1, 1), 4));
		m_titleAnimation.addKeyFrame(new FontColorAnimationKeyframe(m_title, GMVec4(0, 0, 0, 0), 6));
		m_titleAnimation.reset();
		m_titleAnimation.play();

		m_procedures.m_handler->getWorld()->addObjectAndInit(m_title);
		m_procedures.m_handler->getWorld()->addToRenderList(m_title);
	}

	m_titleAnimation.update(dt);
	if (m_titleAnimation.isFinished())
	{
		// TODO 删除m_title，重新创建它，显示它
		m_procedures.m_procedure = Procedures::Scene;
	}

	m_procedures.m_handler->getWorld()->renderScene();
}
