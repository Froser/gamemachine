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
	d->particleSystemManager.reset(new gm::GMParticleSystemManager(getDemoWorldReference()->getContext(), 128));

	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = createDefaultWidget();

	auto top = getClientAreaTop();
	widget->setSize(widget->getSize().width, top + 40);

	gm::GMParticleSystem* psFire = nullptr;
	gm::GMToolUtil::createCocos2DParticleSystem(L"fire1.plist", gm::GMParticleModelType::Particle2D, &psFire, [](auto& description) {
		// 由于采用Ortho视图，坐标系和正常的3D视图相反，y正方向向下，因此我们需要将发射方向旋转180度
		description.setEmitterEmitAngle(description.getEmitterEmitAngle() + 180);
	});
	psFire->getEmitter()->setEmitPosition(GMVec3(800, 600, 0));
	psFire->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psRadius = nullptr;
	gm::GMToolUtil::createCocos2DParticleSystem(L"radius1.plist", gm::GMParticleModelType::Particle2D, &psRadius);
	psRadius->getEmitter()->setEmitPosition(GMVec3(800, 300, 0));
	psRadius->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psStar = nullptr;
	gm::GMToolUtil::createCocos2DParticleSystem(L"star.plist", gm::GMParticleModelType::Particle2D, &psStar);
	psStar->getEmitter()->setEmitPosition(GMVec3(400, 600, 0));
	psStar->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Relative);

	gm::GMParticleSystem* psLeaves = nullptr;
	gm::GMToolUtil::createCocos2DParticleSystem(L"fallenLeaves.plist", gm::GMParticleModelType::Particle2D, &psLeaves);
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
		d->particleSystemManager->update(GM.getRunningStates().lastFrameElpased);
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
	d->particleSystemManager.reset(new gm::GMParticleSystemManager(getDemoWorldReference()->getContext(), 128));

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
		gm::GMToolUtil::createCocos2DParticleSystem(L"fire1.plist", gm::GMParticleModelType::Particle3D, &psFire);
		psFire->getEmitter()->setEmitPosition(position);
		psFire->getEmitter()->getEffect()->setMotionMode(gm::GMParticleMotionMode::Free);
		d->particleSystemManager->addParticleSystem(psFire);
	}
}

void Demo_ParticleBillboard::setLookAt()
{
	D(d);
	d->lookAtRotation = Identity<GMQuat>();

	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	camera.lookAt(s_lookAt);
}

void Demo_ParticleBillboard::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::Update:
		d->particleSystemManager->update(GM.getRunningStates().lastFrameElpased);
		break;
	case gm::GameMachineHandlerEvent::Render:
		d->particleSystemManager->render();
		break;
	case gm::GameMachineHandlerEvent::Activate:
		handleMouseEvent();
		handleDragging();
		break;
	}
}

void Demo_ParticleBillboard::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	if (state.downButton & gm::GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->dragging = true;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(true);
	}
	else if (state.upButton & gm::GMMouseButton_Left)
	{
		d->dragging = false;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(false);
	}
}

void Demo_ParticleBillboard::handleDragging()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();
	if (d->dragging)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;
		gm::GMfloat rotateY = d->mouseDownY - state.posY;
		GMVec3 lookAt3 = Normalize(s_lookAt.lookAt);
		GMVec4 lookAt = GMVec4(lookAt3, 1.f);
		GMQuat q = Rotate(d->lookAtRotation,
			PI * rotateX / windowStates.renderRect.width,
			GMVec3(0, 1, 0));
		d->lookAtRotation = q;
		q = Rotate(d->lookAtRotation,
			PI * rotateY / windowStates.renderRect.width,
			GMVec3(1, 0, 0));
		d->lookAtRotation = q;
		gm::GMCameraLookAt cameraLookAt = {
			GMVec4(s_lookAt.lookAt, 1.f) * QuatToMatrix(q),
			s_lookAt.position
		};
		getDemonstrationWorld()->getContext()->getEngine()->getCamera().lookAt(cameraLookAt);
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
}
