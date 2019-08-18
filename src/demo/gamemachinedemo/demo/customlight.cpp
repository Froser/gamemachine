#include "stdafx.h"
#include "customlight.h"
#include <linearmath.h>
#include <gmwidget.h>
#include <gmgltechnique.h>
#if GM_USE_DX11
#	include <gmdx11technique.h>
#endif
#include <gmlight.h>

namespace
{
	static gm::GMuint32 s_techid;

	template <typename T>
	inline gm::GMsize_t verifyIndicesContainer(Vector<T>& container, gm::IShaderProgram* shaderProgram)
	{
		gm::GMsize_t sz = static_cast<gm::GMGLShaderProgram*>(shaderProgram)->getProgram();
		if (container.size() <= sz)
		{
			static T t = { 0 };
			container.resize(sz + 1, t);
		}
		return sz;
	}

	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(0, 0, 1),
		GMVec3(0, 0, -1)
	);

	class Spotlight : public gm::GMLight
	{
		typedef gm::GMLight Base;
	public:
		enum
		{
			Direction = CustomStart + 1,
			AttenuationConstant,
			AttenuationLinear,
			AttenuationExp,
		};

		virtual bool setLightAttribute3(GMLightAttribute attr, gm::GMfloat* value) override
		{
			switch (attr)
			{
			case Direction:
			{
				this->direction[0] = value[0];
				this->direction[1] = value[1];
				this->direction[2] = value[2];
				this->direction[3] = 1.0f;
				break;
			}
			default:
				return Base::setLightAttribute3(attr, value);
			}
			return true;
		}

		virtual bool setLightAttribute(GMLightAttribute attr, gm::GMfloat value)
		{
			switch (attr)
			{
			case AttenuationConstant:
				attenuation.Constant = value;
				break;
			case AttenuationLinear:
				attenuation.Linear = value;
				break;
			case AttenuationExp:
				attenuation.Exp = value;
				break;
			default:
				return Base::setLightAttribute(attr, value);
			}
			return true;
		}

	protected:
		gm::GMfloat position[4] = { 0, 1, 0, 1 };
		gm::GMfloat color[4];
		gm::GMfloat direction[4] = { 0, -1, 0, 1 };

		struct
		{
			gm::GMfloat Constant = 1.f;
			gm::GMfloat Linear = 0.0f;
			gm::GMfloat Exp = 0.0f;
		} attenuation;

		Vector<gm::GMint32> lightPositionIndices;
		Vector<gm::GMint32> lightDirectionIndices;
	};

	class GLSpotlight : public Spotlight
	{
	public:
		virtual void activateLight(gm::GMuint32 index, gm::ITechnique* technique)
		{
			gm::GMGLTechnique* glTechnique = gm::gm_cast<gm::GMGLTechnique*>(technique);
			gm::IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
			gm::GMString strIndex = gm::GMString((gm::GMint32)index).toStdString();

			gm::GMsize_t shaderIdx = verifyIndicesContainer(lightPositionIndices, shaderProgram);
			if (!lightPositionIndices[shaderIdx])
				lightPositionIndices[shaderIdx] = shaderProgram->getIndex(L"lights[" + strIndex + L"].LightPosition");

			shaderIdx = verifyIndicesContainer(lightDirectionIndices, shaderProgram);
			if (!lightDirectionIndices[shaderIdx])
				lightDirectionIndices[shaderIdx] = shaderProgram->getIndex(L"lights[" + strIndex + L"].LightDirection");

			shaderProgram->setVec3(lightPositionIndices[shaderIdx], position);
			shaderProgram->setVec3(lightDirectionIndices[shaderIdx], direction);
		}
	};

#if GM_USE_DX11
	class DxSpotlight : public Spotlight
	{
	public:
		virtual void activateLight(gm::GMuint32 index, gm::ITechnique* technique)
		{
			gm::GMDx11Technique* dxTechnique = gm::gm_cast<gm::GMDx11Technique*>(technique);
			ID3DX11Effect* effect = dxTechnique->getEffect();
			GM_ASSERT(effect);

			auto lightAttributes = effect->GetVariableByName("lights");
			GM_ASSERT(lightAttributes->IsValid());

			ID3DX11EffectVariable* lightStruct = lightAttributes->GetElement(index);
			GM_ASSERT(lightStruct->IsValid());

			ID3DX11EffectVectorVariable* pPosition = lightStruct->GetMemberByName("LightPosition")->AsVector();
			GM_ASSERT(pPosition->IsValid());
			GM_DX_HR(pPosition->SetFloatVector(position));

			ID3DX11EffectVectorVariable* pLightDirection = lightStruct->GetMemberByName("LightDirection")->AsVector();
			GM_ASSERT(pLightDirection->IsValid());
			GM_DX_HR(pLightDirection->SetFloatVector(direction));
		}
	};
#endif
}

void Demo_CustomLight::init()
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
	model->setTechniqueId(s_techid);
	model->setType(gm::GMModelType::Custom);

	gm::GMTextureAsset tex = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "floor.png");
	gm::GMToolUtil::addTextureToShader(model->getShader(), tex, gm::GMTextureType::Diffuse);
	getDemoWorldReference()->getAssets().addAsset(tex);

	gm::GMAsset quadAsset = getDemoWorldReference()->getAssets().addAsset(asset);
	gm::GMGameObject* obj = new gm::GMGameObject(quadAsset);
	obj->setRotation(Rotate(Radians(80.f), GMVec3(1, 0, 0)));
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", obj);

	gm::GMWidget* widget = createDefaultWidget();
	widget->setSize(widget->getSize().width, getClientAreaTop() + 40);
}

void Demo_CustomLight::event(gm::GameMachineHandlerEvent evt)
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
		getDemoWorldReference()->updateGameWorld(GM.getRunningStates().lastFrameElapsed);
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

void Demo_CustomLight::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
}

void Demo_CustomLight::setDefaultLights()
{
	D(d);
	if (isInited())
	{
#if GM_USE_DX11
		gm::ILight* light = nullptr;
		if (GM.getRunningStates().renderEnvironment == gm::GMRenderEnvironment::OpenGL)
			light = new GLSpotlight();
		else
			light = new DxSpotlight();
#else
		gm::ILight* light = new GLSpotlight();
#endif
		gm::GMfloat lightPos[] = { 0, 2, 0 };
		light->setLightAttribute3(Spotlight::Position, lightPos);
		light->setLightAttribute(Spotlight::AttenuationLinear, .1f);
		Vector<gm::ILight*> lights;
		lights.push_back(light);
		setSpotLights(lights);
	}
}

void Demo_CustomLight::setSpotLights(const Vector<gm::ILight*>& lights)
{
	D(d);
	for (auto& light : lights)
	{
		d->engine->addLight(light);
	}
}

void Demo_CustomLight::initCustomShader(const gm::IRenderContext* context)
{
	gm::IGraphicEngine* engine = context->getEngine();
	gm::GMRenderTechniques techs;
	gm::GMRenderTechnique vertexTech(gm::GMShaderType::Vertex);
	vertexTech.setCode(
		gm::GMRenderEnvironment::OpenGL,
		L"#if GL_ES\n"
		L"precision mediump int;\n"
		L"precision mediump float;\n"
		L"precision mediump sampler2DShadow;\n"
		L"#endif\n"
		L"layout(location = 0) in vec3 position;\n"
		L"layout(location = 1) in vec3 normal;\n"
		L"layout(location = 2) in vec2 uv;\n"
		L"out vec3 _worldPosition;"
		L"out vec2 _uv;"
		L"uniform mat4 GM_ViewMatrix;"
		L"uniform mat4 GM_WorldMatrix;"
		L"uniform mat4 GM_ProjectionMatrix;"
		L"uniform mat4 GM_InverseTransposeModelMatrix;\n"
		L"void main(void)"
		L"{"
		L" _uv = uv;"
		L" _worldPosition = (GM_WorldMatrix * vec4(position, 1)).xyz;"
		L" gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * GM_WorldMatrix * vec4(position, 1);"
		L"}"
	);
	vertexTech.setCode(
		gm::GMRenderEnvironment::DirectX11,
		L"VS_OUTPUT VS_Spotlight(VS_INPUT input)\n"
		L"{\n"
		L"	return VS_3D(input);\n"
		L"}\n"
	);
	techs.addRenderTechnique(vertexTech);

	gm::GMRenderTechnique pixelTech(gm::GMShaderType::Pixel);
	pixelTech.setCode(
		gm::GMRenderEnvironment::OpenGL,
		L"#if GL_ES\n"
		L"precision mediump int;\n"
		L"precision mediump float;\n"
		L"precision mediump sampler2DShadow;\n"
		L"#endif\n"
		L"out vec4 fragColor;\n"
		L"in vec3 _worldPosition;\n"
		L"in vec2 _uv;\n"
		L"uniform vec4 GM_ViewPosition;\n"
		L"uniform int GM_LightCount;\n"
		L"struct GMTexture\n"
		L"{\n"
		L"	sampler2D Texture;\n"
		L"	float OffsetX;\n"
		L"	float OffsetY;\n"
		L"	float ScaleX;\n"
		L"	float ScaleY;\n"
		L"	int Enabled;\n"
		L"};\n"
		L"uniform GMTexture GM_DiffuseTextureAttribute;\n"
		L"struct Light\n"
		L"{\n"
		L"	vec3 LightPosition;\n"
		L"	vec3 LightDirection;\n"
		L"};\n"
		L""
		L"uniform Light lights[10];\n"
		L""
		L"vec4 calcSpotlight(Light light)\n"
		L"{\n"
		L"	vec3 lightToVertex = normalize(_worldPosition - light.LightPosition);\n"
		L"	float spotFactor = dot(lightToVertex, light.LightDirection);\n"
		L"	float cutoff = .9f;\n"
		L"	if (spotFactor > cutoff)\n"
		L"	{\n"
		L"		vec4 color = vec4(1);\n"
		L"		return color * (1.f - (1.f - spotFactor) / (1.f - cutoff));\n"
		L"	}\n"
		L"	return vec4(0);\n"
		L"}\n"
		L"void main(void)\n"
		L"{ \n"
		L"	vec4 totalLight = vec4(0);\n"
		L"	for (int i = 0; i < GM_LightCount; i++)\n"
		L"	{\n"
		L"		totalLight += calcSpotlight(lights[i]);\n"
		L"	}\n"
		L"	fragColor = texture(GM_DiffuseTextureAttribute.Texture, _uv) * totalLight;\n"
		L"}"
	);
	pixelTech.setCode(
		gm::GMRenderEnvironment::DirectX11,
		L"struct SpotlightAttribute\n"
		L"{\n"
		L"	float4 LightPosition;\n"
		L"	float4 LightDirection;\n"
		L"};\n"
		L"SpotlightAttribute lights[10];"
		L"\n"
		L"float4 CalculateSpotlight(PS_INPUT input, SpotlightAttribute light)\n"
		L"{\n"
		L"	float3 lightToVertex = normalize(input.WorldPos.xyz - light.LightPosition.xyz);\n"
		L"	float spotFactor = dot(lightToVertex, light.LightDirection.xyz);\n"
		L"	float cutoff = .9f;\n"
		L"	if (spotFactor > cutoff)\n"
		L"	{\n"
		L"		float4 color = float4(1, 1, 1, 1);\n"
		L"		return color * (1.f - (1.f - spotFactor) / (1.f - cutoff));\n"
		L"	}\n"
		L"	return float4(0, 0, 0, 0);\n"
		L"}\n"
		L"float4 PS_Spotlight(PS_INPUT input) : SV_TARGET\n"
		L"{\n"
		L"	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);\n"
		L"	for (int i = 0; i < GM_LightCount; i++)\n"
		L"	{\n"
		L"		color += CalculateSpotlight(input, lights[i]);\n"
		L"	}\n"
		L"	return color * GM_DiffuseTextureAttribute.Sample(GM_DiffuseTexture, GM_DiffuseSampler, input.Texcoord);\n"
		L"}"
	);
	techs.addRenderTechnique(pixelTech);

	s_techid = engine->getRenderTechniqueManager()->addRenderTechniques(techs);
}
