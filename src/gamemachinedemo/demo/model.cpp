#include "stdafx.h"
#include "model.h"
#include <linearmath.h>
#include <gmmodelreader.h>
#include <gmimagebuffer.h>
#include <gmwidget.h>
#include <gmcontrols.h>

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(0, -.3f, .5f),
		GMVec3(0, .4f, -.5f)
	);
}

void Demo_Model::onDeactivate()
{
	D(d);
	D_BASE(db, Base);
	db->debugConfig.set(gm::GMDebugConfigs::FrameBufferIndex_I32, 0);
	d->skyObject->deactivate();
	DemoHandler::onDeactivate();
}

void Demo_Model::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
	d->lookAtRotation = Identity<GMQuat>();
}

void Demo_Model::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	gm::GMTextureAsset texture = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "bnp.png");
	getDemoWorldReference()->getAssets().addAsset(texture);

	gm::GMGamePackage& pk = *GM.getGamePackageManager();
	gm::GMModelLoadSettings loadSettings(
		"cat/cat.obj",
		getDemoWorldReference()->getContext()
	);

	gm::GMAsset models;
	gm::GMModelReader::load(loadSettings, models);

	for (auto& model : models.getScene()->getModels())
	{
		model.getModel()->getShader().getMaterial().setRefractivity(0.658f);
		model.getModel()->getShader().getMaterial().setAmbient(Zero<GMVec3>());
		model.getModel()->getShader().getMaterial().setDiffuse(Zero<GMVec3>());
		model.getModel()->getShader().getMaterial().setSpecular(Zero<GMVec3>());
	}

	gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(models);
	d->gameObject = new gm::GMGameObject(asset);
	d->gameObject->setTranslation(Translate(GMVec3(0.f, .25f, 0)));
	d->gameObject->setScaling(Scale(GMVec3(.015f, .015f, .015f)));
	d->gameObject->setRotation(Rotate(PI, GMVec3(0, 1, 0)));

	// 创建2个Cube，一个有NormalMap，一个无
	{
		gm::GMSceneAsset cube;
		gm::GMPrimitiveCreator::createCube(gm::GMPrimitiveCreator::one3(), cube);

		gm::GMShader& shader = cube.getScene()->getModels()[0].getModel()->getShader();
		shader.getMaterial().setRefractivity(0.658f);
		shader.getMaterial().setDiffuse(Zero<GMVec3>());
		shader.getMaterial().setSpecular(Zero<GMVec3>());
		shader.getMaterial().setAmbient(Zero<GMVec3>());
		gm::GMToolUtil::addTextureToShader(shader, texture, gm::GMTextureType::NormalMap);

		asset = getDemoWorldReference()->getAssets().addAsset(cube);
		d->gameObject2 = new gm::GMGameObject(asset);
		d->gameObject2->setTranslation(Translate(GMVec3(-0.25f, .25f, 0)));
		d->gameObject2->setScaling(Scale(GMVec3(.1f, .1f, .1f)));
		d->gameObject2->setRotation(Rotate(PI, GMVec3(0, 1, 0)));
	}
	{
		gm::GMSceneAsset cube;
		gm::GMPrimitiveCreator::createCube(gm::GMPrimitiveCreator::one3(), cube);

		gm::GMShader& shader = cube.getScene()->getModels()[0].getModel()->getShader();
		shader.getMaterial().setRefractivity(0.658f);
		shader.getMaterial().setDiffuse(Zero<GMVec3>());
		shader.getMaterial().setSpecular(Zero<GMVec3>());
		shader.getMaterial().setAmbient(Zero<GMVec3>());

		asset = getDemoWorldReference()->getAssets().addAsset(cube);
		d->gameObject3 = new gm::GMGameObject(asset);
		d->gameObject3->setTranslation(Translate(GMVec3(0.25f, .25f, 0)));
		d->gameObject3->setScaling(Scale(GMVec3(.1f, .1f, .1f)));
		d->gameObject3->setRotation(Rotate(PI, GMVec3(0, 1, 0)));
	}

	d->skyObject = createCubeMap();
	d->skyObject->setScaling(Scale(GMVec3(100, 100, 100)));

	asDemoGameWorld(getDemoWorldReference())->addObject("cat", d->gameObject);
	asDemoGameWorld(getDemoWorldReference())->addObject("cube_with_normalmap", d->gameObject2);
	asDemoGameWorld(getDemoWorldReference())->addObject("cube", d->gameObject3);
	asDemoGameWorld(getDemoWorldReference())->addObject("sky", d->skyObject);

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();
	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"交换物体位置",
		10,
		top,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->gameObject2 && d->gameObject3)
		{
			GMMat4 t = d->gameObject2->getTranslation();
			d->gameObject2->setTranslation(d->gameObject3->getTranslation());
			d->gameObject3->setTranslation(t);
		}
	});

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_Model::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	if (state.wheel.wheeled)
	{
		gm::GMfloat delta = .001f * state.wheel.delta / WHEEL_DELTA;
		GMFloat4 scaling;
		{
			GetScalingFromMatrix(d->gameObject->getScaling(), scaling);
			scaling[0] += delta;
			scaling[1] += delta;
			scaling[2] += delta;
			if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
				d->gameObject->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));

			GetScalingFromMatrix(d->gameObject2->getScaling(), scaling);
			d->gameObject2->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));
			scaling[0] += delta * 1.2f;
			scaling[1] += delta * 1.2f;
			scaling[2] += delta * 1.2f;
			if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
			{
				d->gameObject2->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));
				d->gameObject3->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));
			}
		}
	}

	if (state.downButton & gm::GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingL = true;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(true);
	}
	else if (state.upButton & gm::GMMouseButton_Left)
	{
		d->draggingL = false;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(false);
	}
	if (state.downButton & gm::GMMouseButton_Right)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingR = true;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(true);
	}
	else if (state.upButton & gm::GMMouseButton_Right)
	{
		d->draggingR = false;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(false);
	}
}

gm::GMCubeMapGameObject* Demo_Model::createCubeMap()
{
	gm::GMGamePackage* pk = GM.getGamePackageManager();
	gm::GMImage* slices[6] = { nullptr };
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posx.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[0]);
		GM_ASSERT(slices[0]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negx.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[1]);
		GM_ASSERT(slices[1]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posy.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[2]);
		GM_ASSERT(slices[2]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negy.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[3]);
		GM_ASSERT(slices[3]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posz.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[4]);
		GM_ASSERT(slices[4]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negz.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[5]);
		GM_ASSERT(slices[5]);
	}

	gm::GMCubeMapBuffer cubeMap(*slices[0], *slices[1], *slices[2], *slices[3], *slices[4], *slices[5]);
	gm::GMTextureAsset cubeMapTex;
	GM.getFactory()->createTexture(getDemoWorldReference()->getContext(), &cubeMap, cubeMapTex);

	for (auto slice : slices)
	{
		gm::GM_delete(slice);
	}

	return new gm::GMCubeMapGameObject(cubeMapTex);
}

void Demo_Model::handleDragging()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();

	if (d->draggingL)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;

		GMQuat q = Rotate(d->gameObject->getRotation(),
			PI * rotateX / windowStates.renderRect.width,
			GMVec3(0, 1, 0));
		d->gameObject->setRotation(q);
		d->gameObject2->setRotation(q);
		d->gameObject3->setRotation(q);

		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
	else if (d->draggingR)
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

void Demo_Model::setDefaultLights()
{
	D(d);
	if (isInited())
	{
		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
			GM_ASSERT(light);
			gm::GMfloat lightPos[] = { .7f, .7f, -.7f };
			light->setLightAttribute3(gm::GMLight::Position, lightPos);

			gm::GMfloat ambientIntensity[] = { .8f, .8f, .8f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

			gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
	}
}

void Demo_Model::event(gm::GameMachineHandlerEvent evt)
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
	case gm::GameMachineHandlerEvent::Update:
		getDemoWorldReference()->updateGameWorld(GM.getRunningStates().lastFrameElpased);
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
	{
		handleMouseEvent();
		handleDragging();
		break;
	}
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}
