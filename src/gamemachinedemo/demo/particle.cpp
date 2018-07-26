#include "stdafx.h"
#include "particle.h"
#include <gmcontrols.h>
#include <gmwidget.h>

void Demo_Particle::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
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

	d->particleSystemManager->addParticleSystem(psFire);
	d->particleSystemManager->addParticleSystem(psRadius);
}

void Demo_Particle::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
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