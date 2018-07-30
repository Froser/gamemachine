#include "stdafx.h"
#include "md5mesh.h"
#include <gmcontrols.h>
#include <gmwidget.h>
#include <gmmodelreader.h>

void Demo_MD5Mesh::init()
{
	D_BASE(d, Base);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(d->parentDemonstrationWorld->getContext()));

	Base::init();

	auto top = getClientAreaTop();
	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = createDefaultWidget();
	widget->setSize(widget->getSize().width, getClientAreaTop() + 40);

	gm::GMModelReader::load(gm::GMModelLoadSettings(L"boblampclean/boblampclean.md5mesh", L"boblampclean.md5mesh"), gm::GMModelReader::Auto, nullptr);
}

void Demo_MD5Mesh::event(gm::GameMachineHandlerEvent evt)
{
	D_BASE(db, Base);
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Simulate:
		getDemoWorldReference()->simulateGameWorld();
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}
