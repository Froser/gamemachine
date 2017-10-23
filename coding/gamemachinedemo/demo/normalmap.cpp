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

		virtual void onCreateShader(gm::Shader& shader) override
		{
			shader.setCull(gm::GMS_Cull::CULL);
			shader.getMaterial().kd = gm::linear_math::Vector3(.6f, .2f, .3f);
			shader.getMaterial().ks = gm::linear_math::Vector3(.1f, .2f, .3f);
			shader.getMaterial().ka = gm::linear_math::Vector3(1, 1, 1);
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
	gm::GMfloat extents[] = { .5f, .5f, .5f };
	gm::GMfloat pos[] = { 0, 0, -1.f };
	gm::GMModel* model;
	gm::GMPrimitiveCreator::createQuad(extents, pos, &model, &cb);
	gm::GMAsset* quadAsset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, model);
	gm::GMGameObject* obj = new gm::GMGameObject(quadAsset);
	d->demoWorld->appendObject("texture", obj);
}

void Demo_NormalMap::event(gm::GameMachineEvent evt)
{
	D(d);
	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
		d->demoWorld->simulateGameWorld();
		d->demoWorld->notifyControls();
		break;
	case gm::GameMachineEvent::Render:
	{
		d->demoWorld->renderScene();
		break;
	}
	case gm::GameMachineEvent::Activate:
		break;
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}
}