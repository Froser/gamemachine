#include "stdafx.h"
#include "normalmap.h"
#include <linearmath.h>
#include <gmcontrols.h>

void Demo_NormalMap::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();
	
	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	// 创建一个带纹理的对象
	GMVec2 extents = GMVec2(.5f, .5f);
	gm::GMModelAsset asset;
	gm::GMPrimitiveCreator::createQuadrangle(extents, .5f, asset);

	gm::GMModel* model = asset.getScene()->getModels()[0].getModel();
	model->getShader().setCull(gm::GMS_Cull::Cull);
	model->getShader().getMaterial().setDiffuse(GMVec3(.6f, .2f, .3f));
	model->getShader().getMaterial().setSpecular(GMVec3(.1f, .2f, .3f));
	model->getShader().getMaterial().setAmbient(GMVec3(1, 1, 1));
	model->getShader().getMaterial().setShininess(20);

	auto pk = gm::GameMachine::instance().getGamePackageManager();
	gm::GMTextureAsset tex = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "bnp.png");
	gm::GMToolUtil::addTextureToShader(model->getShader(), tex, gm::GMTextureType::NormalMap);
	gm::GMToolUtil::addTextureToShader(model->getShader(), tex, gm::GMTextureType::Diffuse);
	getDemoWorldReference()->getAssets().addAsset(tex);

	gm::GMAsset quadAsset = getDemoWorldReference()->getAssets().addAsset(asset);
	d->gameObject = new gm::GMGameObject(quadAsset);
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", d->gameObject);

	// 为这个对象创建动画
	GMFloat4 t4, s4;
	GetTranslationFromMatrix(d->gameObject->getTranslation(), t4);
	GetScalingFromMatrix(d->gameObject->getScaling(), s4);
	GMVec4 t, s;
	t.setFloat4(t4);
	s.setFloat4(s4);

	d->animation.setTargetObjects(d->gameObject);
	d->animation.addKeyFrame(new gm::GMGameObjectKeyframe(
		t,
		s,
		(Rotate(PI * 2 / 3 , (GMVec3(0, 0, 1)))),
		1.5f
	));
	d->animation.addKeyFrame(new gm::GMGameObjectKeyframe(
		t,
		s,
		(Rotate(PI * 4 / 3, (GMVec3(0, 0, 1)))),
		3.f
	));
	d->animation.addKeyFrame(new gm::GMGameObjectKeyframe(
		t,
		s,
		(Rotate(PI * 2, (GMVec3(0, 0, 1)))),
		4.5f
	));
	d->animation.setPlayLoop(true);

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();

	gm::GMControlLabel* stateLabel = nullptr;
	widget->addControl(stateLabel = gm::GMControlLabel::createControl(
		widget,
		L"状态：旋转中",
		getLabelFontColor(),
		10,
		top,
		250,
		30,
		false
	));
	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"暂停/继续旋转",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->animation.isPlaying())
			d->animation.pause();
		else
			d->animation.play();
		emit(GM_SIGNAL(Demo_NormalMap, rotateStateChanged));
	});

	connect(*this, GM_SIGNAL(Demo_NormalMap, rotateStateChanged), [=](auto, auto) {
		if (d->animation.isPlaying())
			stateLabel->setText(L"状态：旋转中");
		else
			stateLabel->setText(L"状态：已停止");
	});

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_NormalMap::onActivate()
{
	D(d);
	d->animation.play();
	Base::onActivate();
}

void Demo_NormalMap::onDeactivate()
{
	D(d);
	d->animation.pause();
	Base::onDeactivate();
}

void Demo_NormalMap::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	D_BASE(db, Base);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Update:
	{
		getDemoWorldReference()->updateGameWorld(GM.getRunningStates().lastFrameElpased);
		break;
	}
	case gm::GameMachineHandlerEvent::Render:
	{
		d->animation.update(GM.getRunningStates().lastFrameElpased);
		getDemoWorldReference()->renderScene();
		break;
	}
	case gm::GameMachineHandlerEvent::Activate:
	{
		gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		if (kbState.keyTriggered(gm::GM_ASCIIToKey('0')))
			db->debugConfig.set(gm::GMDebugConfigs::FrameBufferIndex_I32, 0);

		for (gm::GMint32 i = 0; i < 8; ++i)
		{
			if (kbState.keyTriggered(gm::GM_ASCIIToKey('1' + (gm::GMint32)i)))
				db->debugConfig.set(gm::GMDebugConfigs::FrameBufferIndex_I32, (gm::GMint32)i + 1);
		}
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