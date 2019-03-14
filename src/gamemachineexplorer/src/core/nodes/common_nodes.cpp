#include "stdafx.h"
#include "common_nodes.h"
#include <core/handler.h>
#include <core/util.h>
#include <gmutilities.h>
#include <gmdiscretedynamicsworld.h>
#include <gmphysicsshape.h>
#include <core/scene_control.h>

namespace
{
	const GMVec3 s_x(1, 0, 0);
	const GMVec3 s_up(0, 1, 0);
	const GMVec3 s_z(0, 0, 1);
}

namespace core
{
	EventResult SplashNode::onMouseMove(const RenderContext& ctx, const RenderMouseDetails& details)
	{
		return ER_Continue;
	}

	void SplashNode::initAsset(const RenderContext& ctx)
	{
		m_asset.asset = createSplash(ctx);
		m_asset.object = new GMGameObject(m_asset.asset);
		ctx.handler->getWorld()->addObjectAndInit(m_asset.object);
	}

	bool SplashNode::hitTest(const RenderContext& ctx)
	{
		return false;
	}

	GMSceneAsset SplashNode::createSplash(const RenderContext& ctx)
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

	EventResult PlaneNode::onMouseMove(const RenderContext& ctx, const RenderMouseDetails& details)
	{
		// 先获取偏移
		float dx = details.position[0] - details.lastPosition[0];
		float dz = details.position[1] - details.lastPosition[1];

		// 如果选中了地面，移动镜头
		// 镜头位置移动
		GMCamera camera = ctx.control->viewCamera();
		GMCameraLookAt lookAt = camera.getLookAt();

		// 镜头位置在相机空间的坐标
		GMVec4 position_Camera = GMVec4(dx, 0, dz, 1);

		// 创建相机坐标系
		GMVec3 cameraZ_World = Normalize(GMVec3(lookAt.lookDirection.getX(), 0, lookAt.lookDirection.getZ()));

		// 求出相机x坐标与世界坐标余弦
		GMVec3 cameraX_World = Normalize(Cross(s_up, cameraZ_World));
		float cos_X = Dot(cameraX_World, s_x);
		float sin_X = Sqrt(1 - cos_X * cos_X);
		GMVec3 posDelta = GMVec3(dx * cos_X + dz * sin_X, 0, dx * sin_X + dz * cos_X);
		lookAt.position = lookAt.position + posDelta;

		camera.lookAt(lookAt);
		ctx.control->setViewCamera(camera);
		ctx.control->setCamera(camera);
		ctx.control->update();
		return ER_OK;
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