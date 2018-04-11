#include "stdafx.h"
#include "texture.h"
#include <linearmath.h>

Demo_Texture::~Demo_Texture()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}

void Demo_Texture::init()
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
			shader.getMaterial().kd = GMVec3(1, 1, 1);
			gm::ITexture* tex = nullptr;
			gm::GMToolUtil::createTexture("gamemachine.png", &tex);
			gm::GMToolUtil::addTextureToShader(shader, tex, gm::GMTextureType::DIFFUSE);
			world->getAssets().insertAsset(gm::GMAssetType::Texture, tex);
		}
	} cb(d->demoWorld);

	// 创建一个带纹理的对象
	gm::GMfloat extents[] = { 1.f, .5f, .5f };
	gm::GMfloat pos[] = { 0, 0, 0 };
	gm::GMModel* model;
	gm::GMPrimitiveCreator::createQuad(extents, pos, &model, &cb);
	gm::GMAsset quadAsset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, model);
	gm::GMGameObject* obj = new gm::GMGameObject(quadAsset);
	d->demoWorld->addObject("texture", obj);
}

void Demo_Texture::event(gm::GameMachineEvent evt)
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
		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		if (kbState.keyTriggered('N'))
			GMSetDebugState(DRAW_NORMAL, (GMGetDebugState(DRAW_NORMAL) + 1) % gm::GMStates_DebugOptions::DRAW_NORMAL_END);
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

void Demo_Texture_Index::init()
{
	D(d);
	Base::init();

	// 创建对象
	d->demoWorld = new gm::GMDemoGameWorld();

	gm::GMModel* quad = new gm::GMModel();
	quad->setPrimitiveTopologyMode(gm::GMTopologyMode::Triangles);
	quad->setDrawMode(gm::GMModelDrawMode::Index);
	gm::GMMesh* mesh = new gm::GMMesh(quad);

	gm::GMVertex V1 = { { -1.f, -.5f, 0 }, { 0, 0, -1 }, { 0, 1 } };
	gm::GMVertex V2 = { { -1.f, .5f, 0 }, { 0, 0, -1 }, { 0, 0 } };
	gm::GMVertex V3 = { { 1.f, -.5f, 0 }, { 0, 0, -1 }, { 1, 1 } };
	gm::GMVertex V4 = { { 1.f, .5f, 0 }, { 0, 0, -1 }, { 1, 0 } };
	mesh->vertex(V1);
	mesh->vertex(V2);
	mesh->vertex(V3);
	mesh->vertex(V4);
	mesh->index(0);
	mesh->index(1);
	mesh->index(2);

	gm::GMMesh* mesh2 = new gm::GMMesh(quad);
	mesh2->vertex(V3);
	mesh2->vertex(V2);
	mesh2->vertex(V4);
	mesh2->index(0);
	mesh2->index(1);
	mesh2->index(2);

	gm::GMShader& shader = quad->getShader();
	shader.getMaterial().kd = GMVec3(1, 1, 1);

	gm::ITexture* tex = nullptr;
	gm::GMToolUtil::createTexture("gamemachine.png", &tex);
	gm::GMToolUtil::addTextureToShader(shader, tex, gm::GMTextureType::DIFFUSE);
	d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Texture, tex);

	gm::GMAsset quadAsset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, quad);
	d->demoWorld->addObject("texture", new gm::GMGameObject(quadAsset));
}