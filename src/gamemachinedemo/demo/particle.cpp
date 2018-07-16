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

	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = createDefaultWidget();

	auto top = getClientAreaTop();
	widget->setSize(widget->getSize().width, top + 40);

	gm::GMParticleSystem* ps = new gm::GMParticleSystem();
	ps->getEmitter()->setEmitPosition(GMVec3(0, 0, 0));
	ps->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);
}