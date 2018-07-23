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
	d->particleSystemManager.reset(new gm::GMParticleSystemManager(getDemoWorldReference()->getContext()));

	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = createDefaultWidget();

	auto top = getClientAreaTop();
	widget->setSize(widget->getSize().width, top + 40);

	gm::GMBuffer buf;
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Particle, "fire1.plist", &buf);
	buf.convertToStringBuffer();

	gm::GMParticleSystem* psFire = new gm::GMParticleSystem();
	psFire->setDescription(gm::GMParticleSystem::createParticleDescriptionFromCocos2DPlist(gm::GMString((const char*)buf.buffer)));
	psFire->getEmitter()->setParticleCount(5);
	psFire->getEmitter()->setEmitPosition(GMVec3(800, 600, 0));
	psFire->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	d->particleSystemManager->addParticleSystem(psFire);
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