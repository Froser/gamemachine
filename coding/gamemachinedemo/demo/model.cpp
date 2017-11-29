#include "stdafx.h"
#include "model.h"
#include <linearmath.h>
#include <gmmodelreader.h>

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

Demo_Model::~Demo_Model()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}

void Demo_Model::setLookAt()
{
	gm::GMCamera& camera = GM.getCamera();
	camera.setPerspective(glm::radians(75.f), 1.333f, .1f, 3200);

	static gm::GMCameraLookAt lookAt = {
		glm::vec3(0, -.3f, -1.f),
		glm::vec3(0, .4f, .5f),
	};
	camera.lookAt(lookAt);
}

void Demo_Model::init()
{
	D(d);
	Base::init();

	// 创建对象
	d->demoWorld = new gm::GMDemoGameWorld();
	
	gm::GMGamePackage& pk = *GM.getGamePackageManager();

	gm::GMModelLoadSettings loadSettings = {
		pk,
		pk.pathOf(gm::GMPackageIndex::Models, "baymax/baymax.obj"),
		"baymax"
	};

	gm::GMModel* model = new gm::GMModel();
	gm::GMModelReader::load(loadSettings, &model);

	// 交给GameWorld管理资源
	gm::GMAsset asset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, model);

	d->gameObject = new gm::GMGameObject(asset);
	d->gameObject->setScaling(glm::scale(.005f, .005f, .005f));
	d->gameObject->setRotation(glm::rotate(glm::identity<glm::quat>(), PI, glm::vec3(0, 1, 0)));

	d->floor = createFloor();
	d->demoWorld->addObject("floor", d->floor);
	d->demoWorld->addObject("baymax", d->gameObject);
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
		}
		{
			glm::getScalingFromMatrix(d->floor->getScaling(), scaling);
			scaling[0] += delta;
			scaling[1] += delta;
			scaling[2] += delta;

			if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
				d->floor->setScaling(glm::scale(scaling[0], scaling[1], scaling[2]));
		}
	}

	if (state.downButton & GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->dragging = true;
	}
	else if (state.upButton & GMMouseButton_Left)
	{
		d->dragging = false;
	}
}

gm::GMGameObject* Demo_Model::createFloor()
{
	D(d);
	// 创建一个纹理
	struct _ShaderCb : public gm::IPrimitiveCreatorShaderCallback
	{
		gm::GMDemoGameWorld* world = nullptr;

		_ShaderCb(gm::GMDemoGameWorld* d) : world(d)
		{
		}

		virtual void onCreateShader(gm::GMShader& shader) override
		{
			shader.setCull(gm::GMS_Cull::CULL);
			shader.getMaterial().kd = glm::vec3(.1f, .2f, .6f);
			shader.getMaterial().ks = glm::vec3(.1f, .2f, .6f);
			shader.getMaterial().ka = glm::vec3(.8f, .8f, 1);
			shader.getMaterial().shininess = 20;

			auto pk = gm::GameMachine::instance().getGamePackageManager();
			auto& container = world->getAssets();

			gm::GMImage* img = nullptr;
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "bnp.png", &buf);
			gm::GMImageReader::load(buf.buffer, buf.size, &img);
			GM_ASSERT(img);

			gm::ITexture* tex = nullptr;
			GM.getFactory()->createTexture(img, &tex);
			GM_ASSERT(tex);
			// 不要忘记释放img
			gm::GM_delete(img);

			world->getAssets().insertAsset(gm::GMAssetType::Texture, tex);
			{
				auto& frames = shader.getTexture().getTextureFrames(gm::GMTextureType::NORMALMAP, 0);
				frames.addFrame(tex);
			}

			{
				auto& frames = shader.getTexture().getTextureFrames(gm::GMTextureType::DIFFUSE, 0);
				frames.addFrame(tex);
			}
		}
	} cb(d->demoWorld);

	// 创建一个带纹理的对象
	gm::GMfloat extents[] = { 1, 1, 1 };
	gm::GMfloat pos[] = { 
		-100, 0, -100,
		-100, 0, 100,
		100, 0, 100,
		100, 0, -100
	};
	gm::GMModel* model;
	gm::GMPrimitiveCreator::createQuad3D(extents, pos, &model, &cb);
	gm::GMAsset quadAsset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, model);
	gm::GMGameObject* obj = new gm::GMGameObject(quadAsset);
	obj->setScaling(glm::scale(.005f, .005f, .005f));
	return obj;
}

void Demo_Model::handleDragging()
{
	D(d);
	if (d->dragging)
	{
		gm::IMouseState& ms = GM.getMainWindow()->getInputMananger()->getMouseState();
		gm::GMMouseState state = ms.mouseState();

		gm::GMfloat rotateX = state.posX - d->mouseDownX;
		{
			glm::quat q = glm::rotate(d->gameObject->getRotation(),
				PI * rotateX / GM.getGameMachineRunningStates().windowRect.width,
				glm::vec3(0, 1, 0));
			d->gameObject->setRotation(q);
		}
		{
			glm::quat q = glm::rotate(d->floor->getRotation(),
				PI * rotateX / GM.getGameMachineRunningStates().windowRect.width,
				glm::vec3(0, 1, 0));
			d->floor->setRotation(q);
		}
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
			gm::GMfloat lightPos[] = { 0, 1.f, -1.f };
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
		handleMouseEvent();
		handleDragging();
		d->demoWorld->notifyControls();
		break;
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}
}
