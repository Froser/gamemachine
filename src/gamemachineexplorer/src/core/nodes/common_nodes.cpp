#include "stdafx.h"
#include "common_nodes.h"
#include <core/handler.h>
#include <core/util.h>
#include <gmutilities.h>
#include <gmdiscretedynamicsworld.h>
#include <gmphysicsshape.h>

namespace core
{
	EventResult LogoNode::onMouseMove(SelectedAssets& selectedAssets, const RenderMouseDetails& details)
	{
		return ER_Continue;
	}

	void LogoNode::initAsset(const RenderContext& ctx)
	{
		m_asset.asset = createLogo(ctx);
		m_asset.object = new GMGameObject(m_asset.asset);
		ctx.handler->getWorld()->addObjectAndInit(m_asset.object);
	}

	GMSceneAsset LogoNode::createLogo(const RenderContext& ctx)
	{
		// 创建一个带纹理的对象
		GMVec2 extents = GMVec2(1.f, .5f);
		GMSceneAsset asset;
		GMPrimitiveCreator::createQuadrangle(extents, 0, asset);

		GMModel* model = asset.getScene()->getModels()[0].getModel();
		model->getShader().getMaterial().setDiffuse(GMVec3(1, 1, 1));
		model->getShader().getMaterial().setSpecular(GMVec3(0));

		GMTextureAsset tex = GMToolUtil::createTexture(ctx.handler->getContext(), "gamemachine.png"); //TODO 考虑从qrc拿
		GMToolUtil::addTextureToShader(model->getShader(), tex, GMTextureType::Diffuse);
		return asset;
	}

	void PlaneNode::initAsset(const RenderContext& ctx)
	{
		constexpr float PLANE_LENGTH = 256.f;
		constexpr float PLANE_WIDTH = 256.f;
		constexpr float HALF_PLANE_LENGTH = PLANE_LENGTH / 2.f;
		constexpr float HALF_PLANE_WIDTH = PLANE_WIDTH / 2.f;
		constexpr float PLANE_HEIGHT = 2.f;

		// 创建一个平面
		GMPlainDescription desc = {
			-HALF_PLANE_LENGTH,
			0,
			-HALF_PLANE_WIDTH,
			PLANE_LENGTH,
			PLANE_WIDTH,
			50,
			50,
			{ 1, 1, 1 }
		};

		utCreatePlain(desc, m_asset.asset);
		m_asset.object = new GMGameObject(m_asset.asset);
		ctx.handler->getWorld()->addObjectAndInit(m_asset.object);

		// 设置一个物理形状
		GMRigidPhysicsObject* rigidPlain = new gm::GMRigidPhysicsObject();
		rigidPlain->setMass(.0f);
		m_asset.object->setPhysicsObject(rigidPlain);
		GMPhysicsShapeHelper::createCubeShape(GMVec3(HALF_PLANE_LENGTH, PLANE_HEIGHT, HALF_PLANE_WIDTH), m_asset.shape);
		rigidPlain->setShape(m_asset.shape);

		// 添加到世界
		ctx.handler->getPhysicsWorld()->addRigidObject(rigidPlain);
	}

}