#include "stdafx.h"
#include "texture.h"
#include <linearmath.h>
#include <gmwidget.h>

void Demo_Texture::init()
{
	D(d);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(d->parentDemonstrationWorld->getContext()));

	// 创建一个带纹理的对象
	GMVec2 extents = GMVec2(1.f, .5f);
	gm::GMSceneAsset asset;
	gm::GMPrimitiveCreator::createQuadrangle(extents, 0, asset);
	
	gm::GMModel* model = asset.getScene()->getModels()[0].getModel();
	model->getShader().getMaterial().kd = GMVec3(1, 1, 1);
	model->getShader().getMaterial().ks = GMVec3(0);

	gm::GMTextureAsset tex = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "gamemachine.png");
	gm::GMToolUtil::addTextureToShader(model->getShader(), tex, gm::GMTextureType::Diffuse);
	getDemoWorldReference()->getAssets().addAsset(tex);

	gm::GMAsset quadAsset = getDemoWorldReference()->getAssets().addAsset(asset);
	gm::GMGameObject* obj = new gm::GMGameObject(quadAsset);
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", obj);

	gm::GMWidget* widget = createDefaultWidget();
	widget->setSize(widget->getSize().width, getClientAreaTop() + 40);
}

void Demo_Texture::event(gm::GameMachineHandlerEvent evt)
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
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
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
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(d->parentDemonstrationWorld->getContext()));

	gm::GMModel* quad = new gm::GMModel();
	quad->setPrimitiveTopologyMode(gm::GMTopologyMode::Triangles);
	quad->setDrawMode(gm::GMModelDrawMode::Index);
	gm::GMPart* mesh = new gm::GMPart(quad);

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

	gm::GMPart* mesh2 = new gm::GMPart(quad);
	mesh2->vertex(V3);
	mesh2->vertex(V2);
	mesh2->vertex(V4);
	mesh2->index(0);
	mesh2->index(1);
	mesh2->index(2);

	gm::GMShader& shader = quad->getShader();
	shader.getMaterial().kd = GMVec3(1, 1, 1);
	shader.getMaterial().ks = GMVec3(0);

	gm::GMTextureAsset tex = gm::GMToolUtil::createTexture(d->demoWorld->getContext(), "gamemachine.png");
	gm::GMToolUtil::addTextureToShader(shader, tex, gm::GMTextureType::Diffuse);
	getDemoWorldReference()->getAssets().addAsset(tex);

	gm::GMAsset quadAsset = getDemoWorldReference()->getAssets().addAsset(gm::GMAsset(gm::GMAssetType::Model, quad));
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", new gm::GMGameObject(quadAsset));
}