#include "stdafx.h"
#include "normalmap.h"
#include <linearmath.h>

Demo_NormalMap::~Demo_NormalMap()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}

void Demo_NormalMap::init()
{
	D(d);
	Base::init();

	// 创建对象
	d->demoWorld = new gm::GMDemoGameWorld();

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
			shader.getMaterial().kd = glm::vec3(.6f, .2f, .3f);
			shader.getMaterial().ks = glm::vec3(.1f, .2f, .3f);
			shader.getMaterial().ka = glm::vec3(1, 1, 1);
			shader.getMaterial().shininess = 20;

			auto pk = gm::GameMachine::instance().getGamePackageManager();
			gm::ITexture* tex = nullptr;
			gm::GMTextureUtil::createTexture("bnp.png", &tex);
			gm::GMTextureUtil::addTextureToShader(shader, tex, gm::GMTextureType::NORMALMAP);
			gm::GMTextureUtil::addTextureToShader(shader, tex, gm::GMTextureType::DIFFUSE);
			world->getAssets().insertAsset(gm::GMAssetType::Texture, tex);
		}
	} cb(d->demoWorld);

	// 创建一个带纹理的对象
	gm::GMfloat extents[] = { .5f, .5f, .5f };
	gm::GMfloat pos[] = { 0, 0, -1.f };
	gm::GMModel* model;
	gm::GMPrimitiveCreator::createQuad(extents, pos, &model, &cb);
	gm::GMAsset quadAsset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, model);
	d->gameObject = new gm::GMGameObject(quadAsset);
	d->demoWorld->addObject("texture", d->gameObject);
}

void Demo_NormalMap::event(gm::GameMachineEvent evt)
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
		d->demoWorld->simulateGameWorld();
		break;
	}
	case gm::GameMachineEvent::Render:
		d->rotation = glm::rotate(glm::identity<glm::quat>(), d->angle, (glm::vec3(0, 0, 1)));
		d->gameObject->setRotation(d->rotation);
		d->demoWorld->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
	{
		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		if (kbState.keyTriggered('P'))
			d->rotate = !d->rotate;
		d->demoWorld->notifyControls();
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