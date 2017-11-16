#include "stdafx.h"
#include "rotation.h"

void Demo_Rotation::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();
	d->gameObject= db->demoWorld->findGameObject("texture");
	d->rotation.setRotation(gm::linear_math::Vector3(0, 0, 1), d->angle);
	d->gameObject->setRotation(d->rotation);
}

void Demo_Rotation::event(gm::GameMachineEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
	{
		if (d->rotate)
			d->angle += .01f;
		break;
	}
	case gm::GameMachineEvent::Render:
	{
		d->rotation.setRotation(gm::linear_math::Vector3(0, 0, 1), d->angle);
		d->gameObject->setRotation(d->rotation);
		break;
	}
	case gm::GameMachineEvent::Activate:
	{
		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		if (kbState.keyTriggered('P'))
			d->rotate = !d->rotate;
		break;
	}
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}
}
