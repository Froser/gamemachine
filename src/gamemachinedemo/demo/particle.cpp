#include "stdafx.h"
#include "particle.h"
#include <gmcontrols.h>
#include <gmwidget.h>

void Demo_Particle::init()
{
	D(d);
	D_BASE(db, Base);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	Base::init();

	// 创建对象
	d->particleSystemManager.reset(new gm::GMParticleSystemManager(getDemoWorldReference()->getContext(), 128));

	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = createDefaultWidget();

	auto top = getClientAreaTop();
	widget->setSize(widget->getSize().width, top + 40);

	gm::GMParticleSystem* psFire = nullptr;
	gm::GMToolUtil::createCocos2DParticleSystem(L"fire1.plist", &psFire);
	psFire->getEmitter()->setEmitPosition(GMVec3(800, 600, 0));
	psFire->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psRadius = nullptr;
	gm::GMToolUtil::createCocos2DParticleSystem(L"radius1.plist", &psRadius);
	psRadius->getEmitter()->setEmitPosition(GMVec3(800, 300, 0));
	psRadius->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psStar = nullptr;
	gm::GMToolUtil::createCocos2DParticleSystem(L"star.plist", &psStar);
	psStar->getEmitter()->setEmitPosition(GMVec3(400, 600, 0));
	psStar->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psLeaves = nullptr;
	gm::GMToolUtil::createCocos2DParticleSystem(L"fallenLeaves.plist", &psLeaves);
	psLeaves->getEmitter()->setEmitPosition(GMVec3(600, 0, 0));
	psLeaves->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	d->particleSystemManager->addParticleSystem(psFire);
	d->particleSystemManager->addParticleSystem(psRadius);
	d->particleSystemManager->addParticleSystem(psStar);
	d->particleSystemManager->addParticleSystem(psLeaves);
}


void Demo_Particle::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	gm::IWindow* window = getDemoWorldReference()->getContext()->getWindow();
	gm::GMRect rc = window->getWindowRect();
	camera.setOrtho(0, rc.width, rc.height, 0, 0, 100);
}

void Demo_Particle::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::Simulate:
		d->particleSystemManager->update(GM.getRunningStates().lastFrameElpased);
		break;
	case gm::GameMachineHandlerEvent::Render:
		d->particleSystemManager->render();
		break;
	}
}