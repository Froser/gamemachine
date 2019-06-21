#if GM_WINDOWS
#include <windows.h>
#endif

#include <gamemachine.h>
#include <gmgl.h>
#include <gmglhelper.h>
#include <gmdiscretedynamicsworld.h>
#include <gmphysicsshape.h>
#include <gmgraphicengine.h>
#include <gmmodelreader.h>
#include <gmlight.h>
#include <wrapper.h>
using namespace gm;

class SimpleHandler : public IGameHandler, IShaderLoadCallback
{
public:
	virtual void init(const IRenderContext* context) override
	{
		m_context = context;

		/************************************************************************/
		/* 先将gm.pk0解压到D:/的gmpk，然后指定游戏资源从这里读取                    */
		/************************************************************************/
		GMGamePackage* pk = GM.getGamePackageManager();
#if GM_WINDOWS
		pk->loadPackage("D:/gmpk");
#else
		pk->loadPackage("/home/froser/Documents/gmpk");
#endif

		/************************************************************************/
		/* 游戏的着色器读取时，回调onLoadShaders                                  */
		/************************************************************************/
		context->getEngine()->setShaderLoadCallback(this);
	}

	virtual void start() override
	{
		/************************************************************************/
		/* 设置摄像机位置                                                        */
		/************************************************************************/
		GMCamera& camera = m_context->getEngine()->getCamera();
		camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);				// 设置一个透视视图

		GMCameraLookAt lookAt;
		lookAt.lookDirection = Normalize(GMVec3(.5f, -.3f, 1));					// 摄像机朝向
		lookAt.position = GMVec3(-1, 2, -3);									// 摄像机位置
		camera.lookAt(lookAt);

		/************************************************************************/
		/* 设置灯光                                                              */
		/************************************************************************/
		{
			ILight* light = nullptr;
			GM.getFactory()->createLight(GMLightType::PointLight, &light);		// 这是一个直接光照
			GM_ASSERT(light);

			GMfloat ambientIntensity[] = { .7f, .7f, .7f };
			light->setLightAttribute3(GMLight::AmbientIntensity, ambientIntensity);

			GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
			light->setLightAttribute3(GMLight::DiffuseIntensity, diffuseIntensity);

			GMfloat lightPos[] = { -3.f, 3.f, -3.f };
			light->setLightAttribute3(GMLight::Position, lightPos);
			light->setLightAttribute(GMLight::AttenuationLinear, .1f);
			m_context->getEngine()->addLight(light);
		}

		/************************************************************************/
		/* 设置阴影                                                              */
		/************************************************************************/
		{
			const GMWindowStates& windowStates = m_context->getWindow()->getWindowStates();
			GMShadowSourceDesc desc;
			desc.position = GMVec4(-3.f, 3.f, -3.f, 1);
			desc.type = GMShadowSourceDesc::CSMShadow;
			desc.camera = m_context->getEngine()->getCamera();
			desc.biasMax = desc.biasMin = 0.0005f;
			desc.width = windowStates.renderRect.width * 2;						// 为了防止锯齿，我们构造个窗口2倍大小的深度缓存
			desc.height = windowStates.renderRect.height * 2;

			GMCameraLookAt lookAt;												// 阴影的投影方向
			desc.camera.lookAt(GMCameraLookAt::makeLookAt(desc.position, GMVec3(0, 0, 0)));
			m_context->getEngine()->setShadowSource(desc);
		}

		/************************************************************************/
		/* 创建游戏世界                                                          */
		/************************************************************************/
		m_world.reset(new GMGameWorld(m_context));

		/************************************************************************/
		/* 创建物理世界，并设置在游戏世界上                                        */
		/************************************************************************/
		GMDiscreteDynamicsWorld* physicsWorld =  new GMDiscreteDynamicsWorld(m_world.get());

		/************************************************************************/
		/* 创建地板。                                                            */
		/* 地板实际上为一个立方体，质量设置为0表示静止（不受重力影响）。              */
		/************************************************************************/
		GMGameObject* ground = new GMGameObject();								// 新建一个游戏对象（地板）
		ground->setTranslation(Translate(GMVec3(0, -50, 0)));					// 游戏对象中心坐标设置为(0, -50, 0)
		GMRigidPhysicsObject* rigidGround = new GMRigidPhysicsObject();			// 新建一个刚体
		ground->setPhysicsObject(rigidGround);									// 将刚体设置在游戏对象（地板）上
		rigidGround->setMass(.0f);												// 质量设置为0，表示不受到重力影响

		/************************************************************************/
		/* 为这个刚体描述物理形状。                                               */
		/* 每一个物理对象由其物理性质和形状(GMPhysicsShape)组成。                  */
		/* 物理属性，如前看到的，有质量，运动状态等。形状则由许多个顶点组成。         */
		/* 因此，对于一个刚体，其形状不会发生变化，但是它的运动状态可能时刻在变。     */
		/* 多个刚体可以共用一个物理形状。                                          */
		/************************************************************************/
		GMPhysicsShapeAsset groundShape;
		GMPhysicsShapeHelper::createCubeShape(GMVec3(50, 50, 50), groundShape);	// 创建一个立方体物理形状
		rigidGround->setShape(m_world->getAssets().addAsset(groundShape));		// 将这个形状添加到世界资产中
		physicsWorld->addRigidObject(rigidGround);							// 将刚体加入物理世界

		/************************************************************************/
		/* 接下来要创建渲染模型。渲染模型仅仅用于渲染，而不会参与物理的计算。         */
		/* 一般而言，渲染模型是和物理形状一致的，它们有相同的顶点和缩放。             */
		/************************************************************************/
		GMModelAsset groundShapeAsset;
		GMPhysicsShapeHelper::createModelFromShape(groundShape.getPhysicsShape(), groundShapeAsset);	// 从物理形状生成一个渲染模型
		GMMaterial& material = groundShapeAsset.getModel()->getShader().getMaterial();		// 设置地板的渲染模型的参数，如ka, kd, ks等，用于phong着色
		material.setAmbient(GMVec3(.8125f / .7f, .644f / .7f, .043f / .7f));
		material.setDiffuse(GMVec3(.1f));
		material.setSpecular(GMVec3(.4f));
		material.setShininess(9);
		ground->setAsset(m_world->getAssets().addAsset(groundShapeAsset));
		m_world->addObjectAndInit(ground);										// 将地板游戏对象加入游戏世界，它将初始化对象（如传递顶点到GPU），并管理其生命周期
		m_world->addToRenderList(ground);										// 将地板游戏对象加入渲染列表

		/************************************************************************/
		/* 从obj文件读取模型，创建其渲染模型                                       */
		/************************************************************************/
		GMPhysicsShapeAsset modelShape;
		GMGamePackage& pk = *GM.getGamePackageManager();
		GMModelLoadSettings loadSettings(
			"teddy/teddy.obj",
			m_world->getContext()
		);

		GMAsset scene;
		bool b = GMModelReader::load(loadSettings, scene);						// 从文件读取模型，创建一个渲染模型
		GM_ASSERT(b);

		/************************************************************************/
		/* 创建物理形状。由于这次是先有渲染模型，因此可以根据渲染模型生成物理模型。    */
		/* 物理形状创建后，用它来生成若干物理对象和游戏对象。                        */
		/* 生成时，注意游戏对象和物理形状两者的缩放要一致。                          */
		/************************************************************************/
		static GMVec3 s_modelScaling(.02f, .02f, .02f);
		static GMVec3 s_colors[4] =
		{
			GMVec3(60.f / 256.f ,186.f / 256.f, 84.f / 256.f),
			GMVec3(244.f / 256.f ,194.f / 256.f, 13.f / 256.f),
		};

		GMPhysicsShapeHelper::createConvexShapeFromTriangleModel(scene, modelShape, false, s_modelScaling);
		GMAsset teddyAsset = m_world->getAssets().addAsset(modelShape);

		for (GMint32 i = 0; i < 2; ++i) // 创建2个对象，一个在上，一个在下
		{
			GMRigidPhysicsObject* rigidBoxObj = new GMRigidPhysicsObject();
			rigidBoxObj->setMass(1.f);

			GMGameObject* gameObject = new GMGameObject();
			gameObject->setTranslation(Translate(GMVec3(0, 2 + .6f*i, 0)));
			gameObject->setScaling(Scale(s_modelScaling));
			gameObject->setPhysicsObject(rigidBoxObj);
			rigidBoxObj->setShape(teddyAsset);

			GMModelAsset m = scene.getScene()->getModels().front();
			GMModel* duplicateModel = new GMModel(m); // 这里用到的是m所指向的渲染模型，m只创建了一次，因此避免了反复创建模型。
			duplicateModel->getShader().getMaterial().setAmbient(s_colors[i % GM_array_size(s_colors)]);
			duplicateModel->getShader().getMaterial().setDiffuse(GMVec3(.1f));
			duplicateModel->getShader().getMaterial().setSpecular(GMVec3(.4f));
			duplicateModel->getShader().getMaterial().setShininess(99);

			gameObject->setAsset(GMAsset(GMAssetType::Model, duplicateModel));
			physicsWorld->addRigidObject(rigidBoxObj);
			m_world->addObjectAndInit(gameObject);
			m_world->addToRenderList(gameObject);
		}
	}

	virtual void event(GameMachineHandlerEvent evt) override
	{
		switch (evt)
		{
		case GameMachineHandlerEvent::Render:
			m_context->getEngine()->getDefaultFramebuffers()->clear();
			m_world->renderScene();
			break;
		case GameMachineHandlerEvent::Update:
			m_world->getPhysicsWorld()->update(GM.getRunningStates().lastFrameElapsed, nullptr);
			break;
		}
	}

	virtual void onLoadShaders(const IRenderContext* context) override
	{
		/************************************************************************/
		/* 从先前指定的游戏包中，读取着色器                                        */
		/************************************************************************/
		if (GM.getRunningStates().renderEnvironment == GMRenderEnvironment::OpenGL)
		{
			GMGLHelper::loadShader(
				context,
				L"gl/main.vert",
				L"gl/main.frag",
				L"gl/deferred/geometry_pass_main.vert",
				L"gl/deferred/geometry_pass_main.frag",
				L"gl/deferred/light_pass_main.vert",
				L"gl/deferred/light_pass_main.frag",
				L"gl/filters/filters.vert",
				L"gl/filters/filters.frag"
			);
		}
		else
		{
			DirectX11LoadShader(context, L"dx11/effect.fx");
		}
	}

private:
	GMOwnedPtr<GMGameWorld> m_world;
	const IRenderContext* m_context = nullptr;
};

#if GM_WINDOWS
int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow
)
{
#elif GM_UNIX
int main(int argc, char* argv[])
{	
#endif

	/************************************************************************/
	/* 设置好工厂类，以及窗口属性                                             */
	/************************************************************************/
	IFactory* factory = nullptr;
	GMRenderEnvironment env = GMRenderEnvironment::DirectX11;
	env = GMCreateFactory(env, GMRenderEnvironment::OpenGL, &factory);
	GM_ASSERT(env != GMRenderEnvironment::Invalid);
	GMWindowDesc mainAttrs;

#if !GM_WINDOWS
	auto hInstance = 0;
#endif
	mainAttrs.instance = hInstance;

	/************************************************************************/
	/* 创建窗口                                                              */
	/************************************************************************/
	IWindow* mainWindow = nullptr;
	factory->createWindow(hInstance, nullptr, &mainWindow);
	mainWindow->create(mainAttrs);
	mainWindow->centerWindow();
	mainWindow->showWindow();
	mainWindow->setHandler(new SimpleHandler());
	GM.addWindow(mainWindow);

	/************************************************************************/
	/* 初始化GameMachine                                                     */
	/************************************************************************/
	gm::GMGameMachineDesc desc;
	desc.factory = factory;
	desc.renderEnvironment = env;
	GM.init(desc);

	/************************************************************************/
	/* 运行GameMachine                                                      */
	/************************************************************************/
	GM.startGameMachine();
	return 0;
}
