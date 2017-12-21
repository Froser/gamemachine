#include "stdafx.h"
#include "model.h"
#include <linearmath.h>
#include <gmmodelreader.h>
#include <gmimagebuffer.h>

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

namespace
{
	const gm::GMCameraLookAt s_lookAt = {
		glm::vec3(0, -.3f, -1.f),
		glm::vec3(0, .4f, .5f),
	};
}

Demo_Model::~Demo_Model()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}


void Demo_Model::onDeactivate()
{
	D(d);
	GMSetDebugState(FRAMEBUFFER_VIEWER_INDEX, 0);
	d->skyObject->deactivate();
	DemoHandler::onDeactivate();
}

void Demo_Model::setLookAt()
{
	D(d);
	gm::GMCamera& camera = GM.getCamera();
	camera.setPerspective(glm::radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
	d->lookAtRotation = glm::identity<glm::quat>();
}

void Demo_Model::init()
{
	D(d);
	Base::init();

	// 创建对象
	d->demoWorld = new gm::GMDemoGameWorld();
	
	gm::GMGamePackage& pk = *GM.getGamePackageManager();
	gm::GMModelLoadSettings loadSettings(
		"cat/cat.obj",
		"cat"
	);

	gm::GMModel* model = new gm::GMModel();
	gm::GMModelReader::load(loadSettings, &model);
	auto& components = model->getMesh()->getComponents();
	for (auto& component : components)
	{
		component->getShader().getMaterial().refractivity = 0.658f;
		component->getShader().getMaterial().ka = component->getShader().getMaterial().kd = component->getShader().getMaterial().ks = glm::vec3(0);
	}

	gm::GMAsset asset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, model);
	d->gameObject = new gm::GMGameObject(asset);
	d->gameObject->setTranslation(glm::translate(glm::vec3(0.f, .25f, 0)));
	d->gameObject->setScaling(glm::scale(.015f, .015f, .015f));
	d->gameObject->setRotation(glm::rotate(glm::identity<glm::quat>(), PI, glm::vec3(0, 1, 0)));

	// 创建2个Cube，一个有NormalMap，一个无
	{
		gm::GMModel* cube = nullptr;
		gm::GMPrimitiveCreator::createCube(gm::GMPrimitiveCreator::unitExtents(), &cube, nullptr);
		auto& cubeComponents = cube->getMesh()->getComponents();
		gm::ITexture* texture = nullptr;
		gm::GMTextureUtil::createTexture("bnp.png", &texture);
		d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Texture, texture);

		for (auto& component : cubeComponents)
		{
			gm::GMShader& shader = component->getShader();
			shader.getMaterial().refractivity = 0.658f;
			shader.getMaterial().kd = shader.getMaterial().ks = shader.getMaterial().ka = glm::vec3(0);

			gm::GMTextureUtil::addTextureToShader(shader, texture, gm::GMTextureType::NORMALMAP);
		}

		asset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, cube);
		d->gameObject2 = new gm::GMGameObject(asset);
		d->gameObject2->setTranslation(glm::translate(glm::vec3(-0.25f, .25f, 0)));
		d->gameObject2->setScaling(glm::scale(.1f, .1f, .1f));
		d->gameObject2->setRotation(glm::rotate(glm::identity<glm::quat>(), PI, glm::vec3(0, 1, 0)));
	}
	{
		gm::GMModel* cube = nullptr;
		gm::GMPrimitiveCreator::createCube(gm::GMPrimitiveCreator::unitExtents(), &cube, nullptr);
		auto& cubeComponents = cube->getMesh()->getComponents();
		gm::ITexture* texture = nullptr;
		gm::GMTextureUtil::createTexture("cube_np.png", &texture);
		d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Texture, texture);

		for (auto& component : cubeComponents)
		{
			gm::GMShader& shader = component->getShader();
			shader.getMaterial().refractivity = 0.658f;
			shader.getMaterial().kd = shader.getMaterial().ks = shader.getMaterial().ka = glm::vec3(0);
		}

		asset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, cube);
		d->gameObject3 = new gm::GMGameObject(asset);
		d->gameObject3->setTranslation(glm::translate(glm::vec3(0.25f, .25f, 0)));
		d->gameObject3->setScaling(glm::scale(.1f, .1f, .1f));
		d->gameObject3->setRotation(glm::rotate(glm::identity<glm::quat>(), PI, glm::vec3(0, 1, 0)));
	}

	d->skyObject = createCubeMap();
	d->skyObject->setScaling(glm::scale(100, 100, 100));

	d->demoWorld->addObject("baymax", d->gameObject);
	d->demoWorld->addObject("cube", d->gameObject2);
	d->demoWorld->addObject("cube_with_normalmap", d->gameObject3);
	d->demoWorld->addObject("sky", d->skyObject);
}

void Demo_Model::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = GM.getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	if (state.wheel.wheeled)
	{
		gm::GMfloat delta = .001f * state.wheel.delta / WHEEL_DELTA;
		gm::GMfloat scaling[4];
		{
			glm::getScalingFromMatrix(d->gameObject->getScaling(), scaling);
			scaling[0] += delta;
			scaling[1] += delta;
			scaling[2] += delta;
			if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
				d->gameObject->setScaling(glm::scale(scaling[0], scaling[1], scaling[2]));

			glm::getScalingFromMatrix(d->gameObject2->getScaling(), scaling);
			d->gameObject2->setScaling(glm::scale(scaling[0], scaling[1], scaling[2]));
			scaling[0] += delta * 1.2f;
			scaling[1] += delta * 1.2f;
			scaling[2] += delta * 1.2f;
			if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
			{
				d->gameObject2->setScaling(glm::scale(scaling[0], scaling[1], scaling[2]));
				d->gameObject3->setScaling(glm::scale(scaling[0], scaling[1], scaling[2]));
			}
		}
	}

	if (state.downButton & GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingL = true;
		GM.getMainWindow()->setLockWindow(true);
	}
	else if (state.upButton & GMMouseButton_Left)
	{
		d->draggingL = false;
		GM.getMainWindow()->setLockWindow(false);
	}
	if (state.downButton & GMMouseButton_Right)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingR = true;
		GM.getMainWindow()->setLockWindow(true);
	}
	else if (state.upButton & GMMouseButton_Right)
	{
		d->draggingR = false;
		GM.getMainWindow()->setLockWindow(false);
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
	gm::ITexture* cubeMapTex = nullptr;
	GM.getFactory()->createTexture(&cubeMap, &cubeMapTex);

	for (auto slice : slices)
	{
		delete slice;
	}

	return new gm::GMCubeMapGameObject(cubeMapTex);
}

void Demo_Model::handleDragging()
{
	D(d);
	gm::IMouseState& ms = GM.getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();

	if (d->draggingL)
	{
		gm::GMfloat rotateX = state.posX - d->mouseDownX;
		gm::GMfloat rotateY = state.posY - d->mouseDownY;

		glm::quat q = glm::rotate(d->gameObject->getRotation(),
			PI * rotateX / GM.getGameMachineRunningStates().windowRect.width,
			glm::vec3(0, 1, 0));
		d->gameObject->setRotation(q);
		d->gameObject2->setRotation(q);
		d->gameObject3->setRotation(q);

		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
	else if (d->draggingR)
	{
		gm::GMfloat rotateX = state.posX - d->mouseDownX;
		gm::GMfloat rotateY = state.posY - d->mouseDownY;
		glm::vec3 lookAt3 = glm::normalize(s_lookAt.lookAt);
		glm::vec4 lookAt = glm::toHomogeneous(lookAt3);
		glm::quat q = glm::rotate(d->lookAtRotation,
			PI * rotateX / GM.getGameMachineRunningStates().windowRect.width,
			glm::vec3(0, 1, 0));
		d->lookAtRotation = q;
		q = glm::rotate(d->lookAtRotation,
			PI * rotateY / GM.getGameMachineRunningStates().windowRect.width,
			glm::vec3(1, 0, 0));
		d->lookAtRotation = q;
		gm::GMCameraLookAt cameraLookAt = {
			glm::mat4_cast(q) * glm::toHomogeneous(s_lookAt.lookAt),
			s_lookAt.position
		};
		GM.getCamera().lookAt(cameraLookAt);
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
			gm::GMLight light(gm::GMLightType::SPECULAR);
			gm::GMfloat lightPos[] = { .7f, .7f, .7f };
			light.setLightPosition(lightPos);
			gm::GMfloat color[] = { .7f, .7f, .7f };
			light.setLightColor(color);
			GM.getGraphicEngine()->addLight(light);
		}

		{
			gm::GMLight light(gm::GMLightType::AMBIENT);
			gm::GMfloat color[] = { .3f, .3f, .3f };
			light.setLightColor(color);
			GM.getGraphicEngine()->addLight(light);
		}
	}
}

void Demo_Model::event(gm::GameMachineEvent evt)
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
		d->demoWorld->simulateGameWorld();
		break;
	case gm::GameMachineEvent::Render:
		d->demoWorld->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
	{
		handleMouseEvent();
		handleDragging();
		d->demoWorld->notifyControls();

		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		if (kbState.keyTriggered('N'))
			GMSetDebugState(DRAW_NORMAL, (GMGetDebugState(DRAW_NORMAL) + 1) % gm::GMStates_DebugOptions::DRAW_NORMAL_END);
		if (kbState.keyTriggered('0'))
			GMSetDebugState(FRAMEBUFFER_VIEWER_INDEX, 0);
		GM_FOREACH_ENUM_CLASS(i, gm::GBufferGeometryType::Position, gm::GBufferGeometryType::EndOfGeometryType)
		{
			if (kbState.keyTriggered('1' + (gm::GMint)i))
				GMSetDebugState(FRAMEBUFFER_VIEWER_INDEX, (gm::GMint)i + 1);
		}
		if (kbState.keyTriggered('S'))
		{
			if (d->gameObject2 && d->gameObject3)
			{
				glm::mat4 t = d->gameObject2->getTranslation();
				d->gameObject2->setTranslation(d->gameObject3->getTranslation());
				d->gameObject3->setTranslation(t);
			}
		}
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
