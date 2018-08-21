#include "stdafx.h"
#include "customshader.h"
#include <linearmath.h>
#include <gmwidget.h>

static gm::GMRenderTechinqueID s_techid;

void Demo_CustomShader::init()
{
	D(d);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(d->parentDemonstrationWorld->getContext()));

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
			shader.getMaterial().ks = GMVec3(0);

			gm::GMTextureAsset tex = gm::GMToolUtil::createTexture(world->getContext(), "gamemachine.png");
			gm::GMToolUtil::addTextureToShader(shader, tex, gm::GMTextureType::Diffuse);
			world->getAssets().addAsset(tex);
		}
	} cb(asDemoGameWorld(getDemoWorldReference()));

	// 创建一个带纹理的对象
	gm::GMfloat extents[] = { 1.f, .5f, .5f };
	gm::GMfloat pos[] = { 0, 0, 0 };
	gm::GMModel* model = nullptr;
	gm::GMPrimitiveCreator::createQuad(extents, pos, &model, &cb);
	
	model->setTechniqueId(s_techid);
	model->setType(gm::GMModelType::Custom);

	gm::GMAsset quadAsset = getDemoWorldReference()->getAssets().addAsset(gm::GMAsset(gm::GMAssetType::Model, model));
	gm::GMGameObject* obj = new gm::GMGameObject(quadAsset);
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", obj);

	gm::GMWidget* widget = createDefaultWidget();
	widget->setSize(widget->getSize().width, getClientAreaTop() + 40);
}

void Demo_CustomShader::event(gm::GameMachineHandlerEvent evt)
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

void Demo_CustomShader::initCustomShader(const gm::IRenderContext* context)
{
	gm::IGraphicEngine* engine = context->getEngine();
	gm::GMRenderTechniques techs;
	gm::GMRenderTechnique vertexTech(gm::GMShaderType::Vertex);
	vertexTech.setCode(
		gm::GMRenderTechniqueEngineType::OpenGL,
		L"#version 410 core\n"
		L"layout(location = 0) in vec3 position;"
		L"layout(location = 1) in vec3 normal;"
		L"layout(location = 2) in vec2 uv;"
		L"layout(location = 3) in vec3 tangent;"
		L"layout(location = 4) in vec3 bitangent;"
		L"layout(location = 5) in vec2 lightmapuv;"
		L"layout(location = 6) in vec4 color;"
		L"out vec2 vs_uv;"
		L"uniform mat4 GM_ViewMatrix;"
		L"uniform mat4 GM_WorldMatrix;"
		L"uniform mat4 GM_ProjectionMatrix;"
		L"void main(void)"
		L"{"
		L" vs_uv = uv;"
		L" gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * GM_WorldMatrix * vec4(position, 1);"
		L"}"
	);
	techs.addRenderTechnique(vertexTech);

	gm::GMRenderTechnique geometryTech(gm::GMShaderType::Geometry);
	geometryTech.setCode(
		gm::GMRenderTechniqueEngineType::OpenGL,
		L"#version 330 core\n														"
		L"layout(triangles) in;														"
		L"layout(triangle_strip, max_vertices = 3) out;								"
		L"in vec2 vs_uv[];															"
		L"out vec2 gs_uv;															"
		L"																			"
		L"void main() {																"
		L"	gs_uv = vs_uv[0];														"
		L"	gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);			"
		L"	EmitVertex();															"
		L"																			"
		L"	gs_uv = vs_uv[1];														"
		L"	gl_Position = gl_in[1].gl_Position + vec4(0.0, 0.1, 0.0, 0.0);			"
		L"	EmitVertex();															"
		L"																			"
		L"	gs_uv = vs_uv[2];														"
		L"	gl_Position = gl_in[2].gl_Position + vec4(-0.1, 0.0, 0.1, 0.0);			"
		L"	EmitVertex();															"
		L"																			"
		L"	EndPrimitive();															"
		L"}																			"
	);
	techs.addRenderTechnique(geometryTech);

	gm::GMRenderTechnique pixelTech(gm::GMShaderType::Pixel);
	pixelTech.setCode(
		gm::GMRenderTechniqueEngineType::OpenGL,
		L"#version 410 core\n"
		L"in vec2 gs_uv;"
		L"struct GMTexture"
		L"{"
		L"	sampler2D Texture;"
		L"	float OffsetX;"
		L"	float OffsetY;"
		L"	float ScaleX;"
		L"	float ScaleY;"
		L"	int Enabled;"
		L"};"
		L"uniform GMTexture GM_DiffuseTextureAttribute;"
		L"void main(void) { gl_FragColor = texture(GM_DiffuseTextureAttribute.Texture, gs_uv); }"
	);
	techs.addRenderTechnique(pixelTech);

	s_techid = engine->getRenderTechniqueManager()->addRenderTechnique(techs);
}