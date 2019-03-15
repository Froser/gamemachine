#include "stdafx.h"
#include "common_nodes.h"
#include <core/handler.h>
#include <core/util.h>
#include <gmutilities.h>
#include <gmdiscretedynamicsworld.h>
#include <gmphysicsshape.h>
#include <core/scene_control.h>
#include <core/scene_model.h>

namespace
{
	const GMVec3 s_x(1, 0, 0);
	const GMVec3 s_up(0, 1, 0);
	const GMVec3 s_z(0, 0, 1);

	GMCamera defaultCamera()
	{
		static std::once_flag s_flag;
		static GMCamera s_camera;
		std::call_once(s_flag, [](GMCamera&) {
			s_camera.setOrtho(-1, 1, -1, 1, .1f, 3200.f);
			GMCameraLookAt lookAt;
			lookAt.lookDirection = { 0, 0, 1 };
			lookAt.position = { 0, 0, -1 };
			s_camera.lookAt(lookAt);
		}, s_camera);
		return s_camera;
	}
}

namespace core
{
	void SplashRenderTree::onRenderTreeSet()
	{
		control()->setCamera(defaultCamera());
	}

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

	void SceneRenderTree::onRenderTreeSet()
	{
		SceneControl* ctrl = control();
		// 重新生成场景相关的资源:
		// 重置摄像机
		IWindow* window = ctrl->handler()->getContext()->getWindow();
		GMRect rc = window->getRenderRect();
		float aspect = rc.width / rc.height;
		GMCamera camera = ctrl->viewCamera();
		camera.setPerspective(Radian(75.f), aspect, .1f, 2000); //TODO ，near和far需要从全局拿
		GlobalProperties& props = ctrl->model()->getProperties();
		GMCameraLookAt lookAt = GMCameraLookAt::makeLookAt(
			GMVec3(props.viewCamera.posX, props.viewCamera.posY, props.viewCamera.posZ),
			GMVec3(props.viewCamera.lookAtX, props.viewCamera.lookAtY, props.viewCamera.lookAtZ));
		camera.lookAt(lookAt);
		ctrl->setViewCamera(camera);
		ctrl->setCamera(camera);
	}

}