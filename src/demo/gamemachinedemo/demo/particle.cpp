#include "stdafx.h"
#include "particle.h"
#include <gmcontrols.h>
#include <gmwidget.h>

void Demo_Particle2D::init()
{
	D(d);
	D_BASE(db, Base);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	Base::init();

	// 创建对象
	d->particleSystemManager.reset(new gm::GMParticleSystemManager_Cocos2D(getDemoWorldReference()->getContext()));

	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = createDefaultWidget();

	auto top = getClientAreaTop();
	widget->setSize(widget->getSize().width, top + 40);

	gm::GMParticleSystem* psFire = nullptr;
	gm::GMParticleSystem::createCocos2DParticleSystem(L"fire1.plist", gm::GMParticleModelType::Particle2D, &psFire, [](auto& description) {
		// 由于采用Ortho视图，坐标系和正常的3D视图相反，y正方向向下，因此我们需要将发射方向旋转180度
		description.setEmitterEmitAngle(description.getEmitterEmitAngle() + 180);
	});
	psFire->getEmitter()->setEmitPosition(GMVec3(800, 600, 0));
	psFire->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psRadius = nullptr;
	gm::GMParticleSystem::createCocos2DParticleSystem(L"radius1.plist", gm::GMParticleModelType::Particle2D, &psRadius);
	psRadius->getEmitter()->setEmitPosition(GMVec3(800, 300, 0));
	psRadius->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psStar = nullptr;
	gm::GMParticleSystem::createCocos2DParticleSystem(L"star.plist", gm::GMParticleModelType::Particle2D, &psStar);
	psStar->getEmitter()->setEmitPosition(GMVec3(400, 600, 0));
	psStar->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psLeaves = nullptr;
	gm::GMParticleSystem::createCocos2DParticleSystem(L"fallenLeaves.plist", gm::GMParticleModelType::Particle2D, &psLeaves);
	psLeaves->getEmitter()->setEmitPosition(GMVec3(600, 0, 0));
	psLeaves->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	d->particleSystemManager->addParticleSystem(psFire);
	d->particleSystemManager->addParticleSystem(psRadius);
	d->particleSystemManager->addParticleSystem(psStar);
	d->particleSystemManager->addParticleSystem(psLeaves);
}

void Demo_Particle2D::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	gm::IWindow* window = getDemoWorldReference()->getContext()->getWindow();
	gm::GMRect rc = window->getWindowRect();
	camera.setOrtho(0, rc.width, rc.height, 0, 0, 100);
}

void Demo_Particle2D::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::Update:
		d->particleSystemManager->update(GM.getRunningStates().lastFrameElapsed);
		break;
	case gm::GameMachineHandlerEvent::Render:
		d->particleSystemManager->render();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(.86f, 0, .5f),
		GMVec3(0, 0, 0)
	);
}

void Demo_ParticleBillboard::init()
{
	D(d);
	D_BASE(db, Base);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	Base::init();

	// 创建对象
	d->particleSystemManager.reset(new gm::GMParticleSystemManager_Cocos2D(getDemoWorldReference()->getContext()));

	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = createDefaultWidget();

	auto top = getClientAreaTop();
	widget->setSize(widget->getSize().width, top + 40);

	gm::GMParticleSystem* psFire = nullptr;

	GMVec3 positions[] = {
		GMVec3(860, 250, 500),
		GMVec3(860, 250, -500),
		GMVec3(-860, 250, 500),
		GMVec3(-860, 250, -500),
		GMVec3(860, -250, 500),
		GMVec3(860, -250, -500),
		GMVec3(-860, -250, 500),
		GMVec3(-860, -250, -500),
	};

	for (auto position : positions)
	{
		gm::GMParticleSystem::createCocos2DParticleSystem(L"fire1.plist", gm::GMParticleModelType::Particle3D, &psFire);
		psFire->getEmitter()->setEmitPosition(position);
		psFire->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Free);
		d->particleSystemManager->addParticleSystem(psFire);
	}
}

void Demo_ParticleBillboard::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	camera.lookAt(s_lookAt);

	d->cameraUtility.setCamera(&camera);
}

void Demo_ParticleBillboard::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::Update:
		d->particleSystemManager->update(GM.getRunningStates().lastFrameElapsed);
		break;
	case gm::GameMachineHandlerEvent::Render:
		d->particleSystemManager->render();
		break;
	case gm::GameMachineHandlerEvent::Activate:
		handleMouseEvent();
		break;
	}
}

void Demo_ParticleBillboard::onActivate()
{
	D(d);
	d->activated = true;
	gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
	gm::IMouseState& mouseState = inputManager->getMouseState();
	mouseState.setDetectingMode(true);
	Base::onActivate();
}

void Demo_ParticleBillboard::onDeactivate()
{
	D(d);
	d->activated = false;
	gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
	gm::IMouseState& mouseState = inputManager->getMouseState();
	mouseState.setDetectingMode(false);
	Base::onDeactivate();
}

void Demo_ParticleBillboard::handleMouseEvent()
{
	D(d);
	if (d->activated)
	{
		const static gm::GMfloat mouseSensitivity = 0.25f;
		gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
		gm::IMouseState& mouseState = inputManager->getMouseState();
		auto ms = mouseState.state();
		d->cameraUtility.update(Radian(-ms.deltaX * mouseSensitivity), Radian(-ms.deltaY * mouseSensitivity));
	}
}