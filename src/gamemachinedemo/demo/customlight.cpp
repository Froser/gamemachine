#include "stdafx.h"
#include "customlight.h"
#include <linearmath.h>
#include <gmwidget.h>
#include <gmgltechnique.h>
#include <gmlight.h>

namespace
{
	static gm::GMuint32 s_techid;
	template <typename CharType, size_t ArraySize>
	void combineUniform(REF CharType(&dest)[ArraySize], const char* srcA, const char* srcB)
	{
		gm::GMString::stringCopy(dest, srcA);
		gm::GMString::stringCat(dest, srcB);
	}

	template <typename CharType, size_t ArraySize>
	void combineUniform(REF CharType(&dest)[ArraySize], const char* srcA, const char* srcB, const char* srcC)
	{
		gm::GMString::stringCopy(dest, srcA);
		gm::GMString::stringCat(dest, srcB);
		gm::GMString::stringCat(dest, srcC);
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
				return setLightAttribute3(attr, value);
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
	};

	class GLSpotLight : public Spotlight
	{
	public:
		virtual void activateLight(gm::GMuint32 index, gm::ITechnique* technique)
		{
			constexpr gm::GMint32 GMGL_MAX_UNIFORM_NAME_LEN = 64; //uniform最长名称
			static char light_Position[GMGL_MAX_UNIFORM_NAME_LEN];
			static char light_Color[GMGL_MAX_UNIFORM_NAME_LEN];
			static char light_Direction[GMGL_MAX_UNIFORM_NAME_LEN];
			static char light_Attenuation_Constant[GMGL_MAX_UNIFORM_NAME_LEN];
			static char light_Attenuation_Linear[GMGL_MAX_UNIFORM_NAME_LEN];
			static char light_Attenuation_Exp[GMGL_MAX_UNIFORM_NAME_LEN];

			gm::GMGLTechnique* glTechnique = gm::gm_cast<gm::GMGLTechnique*>(technique);
			gm::IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
			std::string stdStrIdx = gm::GMString((gm::GMint32)index).toStdString();
			const char* strIndex = stdStrIdx.c_str();

			combineUniform(light_Color, "GM_lights[", strIndex, "].LightColor");
			shaderProgram->setVec3(light_Color, color);

			combineUniform(light_Position, "GM_lights[", strIndex, "].LightPosition");
			shaderProgram->setVec3(light_Position, position);

			combineUniform(light_Direction, "GM_lights[", strIndex, "].LightDirection");
			shaderProgram->setVec3(light_Direction, direction);

			combineUniform(light_Attenuation_Constant, "GM_lights[", strIndex, "].Attenuation.Constant");
			shaderProgram->setFloat(light_Attenuation_Constant, attenuation.Constant);

			combineUniform(light_Attenuation_Linear, "GM_lights[", strIndex, "].Attenuation.Linear");
			shaderProgram->setFloat(light_Attenuation_Linear, attenuation.Linear);

			combineUniform(light_Attenuation_Exp, "GM_lights[", strIndex, "].Attenuation.Exp");
			shaderProgram->setFloat(light_Attenuation_Exp, attenuation.Exp);
		}
	};
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
		if (GM.getRunningStates().renderEnvironment == gm::GMRenderEnvironment::OpenGL)
		{
			gm::ILight* light = new GLSpotLight();
			gm::GMfloat lightPos[] = { 0, 2, 0 };
			light->setLightAttribute3(GLSpotLight::Position, lightPos);
			gm::GMfloat color[] = { .0f, .9f, .9f };
			light->setLightAttribute3(GLSpotLight::Color, color);
			light->setLightAttribute(GLSpotLight::AttenuationLinear, .1f);
			Vector<gm::ILight*> lights;
			lights.push_back(light);
			setSpotLights(lights);
		}
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
		L"#version 330 core\n"
		L"layout(location = 0) in vec3 position;\n"
		L"layout(location = 1) in vec3 normal;\n"
		L"layout(location = 2) in vec2 uv;\n"
		L"out vec3 _worldPosition;"
		L"out vec3 _normal;"
		L"out vec2 _uv;"
		L"uniform mat4 GM_ViewMatrix;"
		L"uniform mat4 GM_WorldMatrix;"
		L"uniform mat4 GM_ProjectionMatrix;"
		L"uniform mat4 GM_InverseTransposeModelMatrix;\n"
		L"void main(void)"
		L"{"
		L" _uv = uv;"
		L" mat3 inverse_transpose_model_matrix = mat3(GM_InverseTransposeModelMatrix);"
		L" _normal = inverse_transpose_model_matrix * normal;"
		L" _worldPosition = (GM_WorldMatrix * vec4(position, 1)).xyz;"
		L" gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * GM_WorldMatrix * vec4(position, 1);"
		L"}"
	);
	techs.addRenderTechnique(vertexTech);

	gm::GMRenderTechnique pixelTech(gm::GMShaderType::Pixel);
	pixelTech.setCode(
		gm::GMRenderEnvironment::OpenGL,
		L"#version 330 core\n"
		L"in vec3 _worldPosition;\n"
		L"in vec3 _normal;\n"
		L"in vec2 _uv;\n"
		L"uniform vec4 GM_ViewPosition;\n"
		L"uniform int GM_LightCount = 0;\n"
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
		L"struct Attenuation_t\n"
		L"{"
		L"	float Constant;\n"
		L"	float Linear;\n"
		L"	float Exp;\n"
		L"};\n"
		L""
		L"struct Light\n"
		L"{\n"
		L"	vec3 LightColor;\n"
		L"	vec3 LightPosition;\n"
		L"	vec3 LightDirection;\n"
		L"	Attenuation_t Attenuation;\n"
		L"};\n"
		L""
		L"uniform Light GM_lights[50];\n"
		L""
		L"vec4 calcLight(Light light, vec3 lightDirection, vec3 normal)\n"
		L"{\n"
		L"	float diffuseFactor = dot(normal, -lightDirection);\n"
		L"	vec4 diffuseColor = vec4(0);\n"
		L"	vec4 specularColor = vec4(0);\n"
		L"	if (diffuseFactor > 0)\n"
		L"	{\n"
		L"		diffuseColor = vec4(light.LightColor * diffuseFactor, 1.f);\n"
		L""
		L"		vec3 vertexToEye = normalize(GM_ViewPosition.xyz - _worldPosition);\n"
		L"		vec3 lightReflect = normalize(reflect(lightDirection, normal));\n"
		L"		float specularFactor = dot(vertexToEye, lightReflect);\n"
		L"		if (specularFactor > 0)\n"
		L"		{\n"
		L"			specularFactor *= pow(specularFactor, 99);\n"
		L"			specularColor = vec4(light.LightColor * .5f * specularFactor, 1.f);\n"
		L"		}\n"
		L"	}\n"
		L"	return diffuseColor + specularColor;\n"
		L"}\n"
		L""
		L"vec4 calcPointLight(Light light, vec3 normal)\n"
		L"{"
		L"	vec3 lightDirection = _worldPosition - light.LightPosition;\n"
		L"	float distance = length(lightDirection);\n"
		L"	lightDirection = normalize(lightDirection);\n"
		L"	vec4 color = calcLight(light, lightDirection, normal);\n"
		L"	float attenuation = light.Attenuation.Constant +\n"
		L"						light.Attenuation.Linear * distance +\n"
		L"						light.Attenuation.Exp * distance * distance;"
		L"	return color / attenuation;\n"
		L"}"
		L""
		L"vec4 calcSpotlight(Light light, vec3 normal)\n"
		L"{\n"
		L"	vec3 lightToVertex = normalize(_worldPosition - light.LightPosition);\n"
		L"	float spotFactor = dot(lightToVertex, light.LightDirection);\n"
		L"	float cutoff = .2f;\n"
		L"	if (spotFactor > cutoff)\n"
		L"	{\n"
		L"		vec4 color = calcPointLight(light, normal);\n"
		L"		return color * (1.f - (1.f - spotFactor) / (1.f - cutoff));\n"
		L"	}\n"
		L"	return vec4(0);\n"
		L"}\n"
		L"void main(void)\n"
		L"{ \n"
		L"	vec4 totalLight = vec4(0);\n"
		L"	for (int i = 0; i < GM_LightCount; i++)\n"
		L"	{\n"
		L"		totalLight += calcSpotlight(GM_lights[i], _normal);\n"
		L"	}\n"
		L"	gl_FragColor = texture(GM_DiffuseTextureAttribute.Texture, _uv) * totalLight;\n"
		L"}"
	);
	techs.addRenderTechnique(pixelTech);

	s_techid = engine->getRenderTechniqueManager()->addRenderTechniques(techs);
}