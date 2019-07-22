#include "stdafx.h"
#include "timeline.h"
#include <gmimagebuffer.h>
#include <gmimage.h>
#include <gmgameobject.h>
#include <gmlight.h>
#include <gmmodelreader.h>
#include <gmutilities.h>
#include <gmgraphicengine.h>
#include <gmparticle.h>
#include <gmm.h>
#include "helper.h"

#define NoCameraComponent 0
#define PositionComponent 0x01
#define DirectionComponent 0x02
#define FocusAtComponent 0x04

#define PerspectiveComponent 0x01
#define CameraLookAtComponent 0x02
#define OrthoComponent 0x04

template <typename T>
struct AssetCast_
{
	typedef AutoReleasePtr<T>* Type;
};

template <typename T> auto asset_cast(void* obj)
{
	return static_cast<typename AssetCast_<T>::Type>(obj);
}

template <> auto asset_cast<GMCamera>(void* obj)
{
	return static_cast<GMCamera*>(obj);
}

template <> auto asset_cast<GMShadowSourceDesc>(void* obj)
{
	return static_cast<GMShadowSourceDesc*>(obj);
}

struct CameraParams
{
	union
	{
		struct
		{
			GMfloat left, right, top, bottom;
		};

		struct
		{
			GMfloat fovy, aspect;
		};
	};
	GMfloat n, f;
};

template<class T>
struct Nullable
{
	T value;
	bool isNull;

	Nullable() : isNull(true) {}
	Nullable(const T& val) : value(val), isNull(false) {}
	Nullable& operator=(T&& val) { isNull = false; value = std::forward<T>(val); return *this; }
	operator T&() { return value; }
	operator const T&() const { return value; }
};

namespace
{
	template <typename First>
	bool isAny(First first)
	{
		return !!first;
	}

	template <typename First, typename... Others>
	bool isAny(First first, Others... other)
	{
		return !!first || isAny(other...);
	}

	template <typename P, typename First, typename... Others>
	void callEvery_1(P&& param, First first)
	{
		if (first)
			first(std::forward<P>(param));
	}

	template <typename P, typename First, typename... Others>
	void callEvery_1(P&& param, First first, Others... other)
	{
		if (first)
			first(std::forward<P>(param));

		callEvery_1(std::forward<P>(param), other...);
	}

	template <AssetType Type, typename Container>
	bool getAssetAndType(Container& container, const GMString& objectName, REF AssetType& result, OUT void** out)
	{
		result = AssetType::NotFound;

		decltype(container.end()) iter;
		if ((iter = container.find(objectName)) != container.end())
		{
			result = Type;
			if (out)
				(*out) = &(iter->second);
			return true;
		}

		return false;
	}

	bool toBool(const GMString& str)
	{
		if (str == L"true")
			return true;

		if (str == L"false")
			return false;

		gm_warning(gm_dbg_wrap("Unrecognized boolean string '{0}', treat as false."), str);
		return false;
	}

	GMS_BlendFunc toBlendFunc(const GMString& str)
	{
		GMS_BlendFunc result = GMS_BlendFunc::One;
		if (str == L"Zero")
			result = GMS_BlendFunc::Zero;
		else if (str == L"One")
			result = GMS_BlendFunc::One;
		else if (str == L"SourceColor")
			result = GMS_BlendFunc::SourceColor;
		else if (str == L"DestColor")
			result = GMS_BlendFunc::DestColor;
		else if (str == L"SourceAlpha")
			result = GMS_BlendFunc::SourceAlpha;
		else if (str == L"DestAlpha")
			result = GMS_BlendFunc::DestAlpha;
		else if (str == L"OneMinusSourceAlpha")
			result = GMS_BlendFunc::OneMinusSourceAlpha;
		else if (str == L"OneMinusDestAlpha")
			result = GMS_BlendFunc::OneMinusDestAlpha;
		else if (str == L"OneMinusSourceColor")
			result = GMS_BlendFunc::OneMinusSourceColor;
		else if (str == L"OneMinusDestColor")
			result = GMS_BlendFunc::OneMinusDestColor;
		else
			gm_warning(gm_dbg_wrap("Unrecognized blend function string '{0}', treat as One."), str);
		return result;
	}
}

enum Animation
{
	Camera,
	Light,
	GameObject,
	EndOfAnimation,
};

AnimationContainer::AnimationContainer()
	: m_playingAnimationIndex(0)
	, m_editingAnimationIndex(-1)
{
}

void AnimationContainer::newAnimation()
{
	m_animations.resize(m_animations.size() + 1);
}

void AnimationContainer::playAnimation()
{
	m_animations[m_playingAnimationIndex].play();
}

void AnimationContainer::pauseAnimation()
{
	m_animations[m_playingAnimationIndex].pause();
}

void AnimationContainer::updateAnimation(GMfloat dt)
{
	m_animations[m_playingAnimationIndex].update(dt);
}

GMAnimation& AnimationContainer::currentEditingAnimation()
{
	return m_animations[m_editingAnimationIndex];
}

void AnimationContainer::nextEditAnimation()
{
	++m_editingAnimationIndex;
	if (gm_sizet_to_int(m_animations.size()) >= m_editingAnimationIndex)
		newAnimation();
}

void AnimationContainer::nextPlayingAnimation()
{
	++m_playingAnimationIndex;
	if (gm_sizet_to_int(m_animations.size()) >= m_playingAnimationIndex)
		newAnimation();
}

Timeline::Timeline(const IRenderContext* context, GMGameWorld* world)
	: m_context(context)
	, m_world(world)
	, m_timeline(0)
	, m_playing(false)
	, m_finished(false)
	, m_lastTime(0)
	, m_checkpointTime(0)
	, m_audioPlayer(gmm::GMMFactory::getAudioPlayer())
	, m_audioReader(gmm::GMMFactory::getAudioReader())
{
	GM_ASSERT(m_context && m_world);
	m_animations.resize(EndOfAnimation);
	m_world->setParticleSystemManager(new GMParticleSystemManager(m_context));
}

Timeline::~Timeline()
{
}

bool Timeline::parse(const GMString& timelineContent)
{
	initPresetConstants();

	GMXMLDocument doc;
	std::string content = timelineContent.toStdString();
	if (GMXMLError::XML_SUCCESS == doc.Parse(content.c_str()))
	{
		auto root = doc.RootElement();
		if (GMString::stringEquals(root->Name(), "timeline"))
		{
			auto firstElement = root->FirstChildElement();
			parseElements(firstElement);
			return true;
		}
	}
	else
	{
		gm_warning(gm_dbg_wrap("Parse timeline file error: {0}, {1}, {2}"), GMString(doc.ErrorLineNum()), GMString(doc.ErrorName()), GMString(doc.ErrorStr()));
	}
	return false;
}

bool Timeline::parseComponent(const GMString& componentContent)
{
	GMXMLDocument doc;
	std::string content = componentContent.toStdString();
	if (GMXMLError::XML_SUCCESS == doc.Parse(content.c_str()))
	{
		auto root = doc.RootElement();
		if (GMString::stringEquals(root->Name(), "component"))
		{
			auto firstElement = root->FirstChildElement();
			parseElements(firstElement);
			return true;
		}
	}
	else
	{
		gm_warning(gm_dbg_wrap("Parse component file error: {0}, {1}, {2}"), GMString(doc.ErrorLineNum()), GMString(doc.ErrorName()), GMString(doc.ErrorStr()));
	}
	return false;
}


void Timeline::update(GMDuration dt)
{
	if (m_playing)
	{
		if (m_timeline == 0)
			runImmediateActions();
		else
			runActions();

		m_timeline += dt;
	}

	for (auto& animationList : m_animations)
	{
		for (auto& animation : animationList)
		{
			animation.second.updateAnimation(dt);
		}
	}
}

void Timeline::play()
{
	m_currentAction = m_deferredActions.begin();
	m_playing = true;
}

GMString Timeline::getValueFromDefines(GMString id)
{
	GMString sign;
	if (id.startsWith("+") || id.startsWith("-"))
	{
		sign = id[0];
		id = id.substr(1, id.length() - 1);
	}

	if (id.startsWith("$"))
	{
		GMString key = id.substr(1, id.length() - 1);
		auto replacementIter = m_defines.find(key);
		if (replacementIter != m_defines.end())
			return sign + replacementIter->second;

		replacementIter = m_presetConstants.find(key);
		if (replacementIter != m_presetConstants.end())
			return sign + replacementIter->second;
	}
	else if (id.startsWith("@"))
	{
		GMString key = id.substr(1, id.length() - 1);
		GMString result;
		if (m_slots.getSlotByName(key, result))
			return sign + result;
	}

	return sign + id;
}

void Timeline::pause()
{
	m_playing = false;
	for (auto& animationList : m_animations)
	{
		for (auto& animation : animationList)
		{
			animation.second.pauseAnimation();
		}
	}
}

void Timeline::initPresetConstants()
{
	// 写入一些常量
	const GMRect& winRc = m_context->getWindow()->getRenderRect();
	m_presetConstants["GM_screenWidth"] = GMString(winRc.width);
	m_presetConstants["GM_screenHeight"] = GMString(winRc.height);
}

void Timeline::parseElements(GMXMLElement* e)
{
	while (e)
	{
		GMString name = e->Name();
		if (name == L"defines")
		{
			parseDefines(e->FirstChildElement());
		}
		else if (name == L"assets")
		{
			parseAssets(e->FirstChildElement());
		}
		else if (name == L"objects")
		{
			parseObjects(e->FirstChildElement());
		}
		else if (name == L"actions")
		{
			parseActions(e->FirstChildElement());
		}

		e = e->NextSiblingElement();
	}
}

void Timeline::parseDefines(GMXMLElement* e)
{
	while (e)
	{
		GMString text = getValueFromDefines(e->GetText());
		auto result = m_defines.insert(std::make_pair(e->Name(), text));
		if (!result.second)
			gm_warning(gm_dbg_wrap("The define name '{0}' has been already taken."), e->Name());
		e = e->NextSiblingElement();
	}
}

void Timeline::parseAssets(GMXMLElement* e)
{
	auto package = GM.getGamePackageManager();
	while (e)
	{
		GMString id = e->Attribute("id");
		GMString name = e->Name();
		if (name == L"cubemap")
		{
			GMBuffer l, r, u, d, f, b;
			package->readFile(GMPackageIndex::Textures, e->Attribute("left"), &l);
			package->readFile(GMPackageIndex::Textures, e->Attribute("right"), &r);
			package->readFile(GMPackageIndex::Textures, e->Attribute("up"), &u);
			package->readFile(GMPackageIndex::Textures, e->Attribute("down"), &d);
			package->readFile(GMPackageIndex::Textures, e->Attribute("front"), &f);
			package->readFile(GMPackageIndex::Textures, e->Attribute("back"), &b);
			if (l.getSize() > 0 && r.getSize() > 0 && f.getSize() > 0 && b.getSize() > 0 && u.getSize() > 0 && d.getSize() > 0)
			{
				GMImage *imgl, *imgr, *imgu, *imgd, *imgf, *imgb;
				if (GMImageReader::load(l.getData(), l.getSize(), &imgl) &&
					GMImageReader::load(r.getData(), r.getSize(), &imgr) &&
					GMImageReader::load(u.getData(), u.getSize(), &imgu) &&
					GMImageReader::load(d.getData(), d.getSize(), &imgd) &&
					GMImageReader::load(f.getData(), f.getSize(), &imgf) &&
					GMImageReader::load(b.getData(), b.getSize(), &imgb))
				{
					GMCubeMapBuffer cubeMap(*imgr, *imgl, *imgu, *imgd, *imgf, *imgb);
					GMTextureAsset cubeMapTex;
					GM.getFactory()->createTexture(m_context, &cubeMap, cubeMapTex);
					GM_delete(imgl);
					GM_delete(imgr);
					GM_delete(imgu);
					GM_delete(imgd);
					GM_delete(imgf);
					GM_delete(imgb);
					m_assets[id] = cubeMapTex;
				}
				else
				{
					gm_warning(gm_dbg_wrap("Load image failed. Id: {0}"), id);
				}
			}
			else
			{
				gm_warning(gm_dbg_wrap("Load asset failed. Id: {0}"), id);
			}
		}
		else if (name == L"texture")
		{
			GMString file = e->Attribute("file");
			GMTextureAsset asset = GMToolUtil::createTexture(m_context, file);
			if (asset.isEmpty())
			{
				gm_warning(gm_dbg_wrap("Load asset failed. Id: {0}"), id);
			}
			else
			{
				m_assets[id] = asset;
			}
		}
		else if (name == L"pbr")
		{
			GMString albedo = e->Attribute("albedo");
			GMString metallic = e->Attribute("metallic");
			GMString roughness = e->Attribute("roughness");
			GMString ao = e->Attribute("ao");
			GMString normal = e->Attribute("normal");
			PBR pbr;
			bool b = GMToolUtil::createPBRTextures(
				m_context,
				albedo,
				metallic,
				roughness,
				ao,
				normal,
				pbr.albedo,
				pbr.metallicRoughnessAO,
				pbr.normal
			);
			if (!b)
			{
				gm_warning(gm_dbg_wrap("Create PBR texture failed: {0}"), id);
			}
			else
			{
				m_pbrs[id] = pbr;
			}
		}
		else if (name == L"object")
		{
			GMString file = e->Attribute("file");
			GMModelLoadSettings loadSettings(
				file,
				m_context
			);
			GMSceneAsset asset;
			if (GMModelReader::load(loadSettings, asset))
			{
				m_assets[id] = asset;
			}
			else
			{
				gm_warning(gm_dbg_wrap("Load asset failed: {0}"), loadSettings.filename);
			}
		}
		else if (name == L"buffer")
		{
			GMString file = e->Attribute("file");
			GMBuffer buffer;
			package->readFile(GMPackageIndex::Root, file, &buffer);
			if (buffer.getSize() > 0)
			{
				m_buffers[id] = buffer;
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot get file: {0}"), file);
			}
		}
		else if (name == L"audio")
		{
			GMString file = e->Attribute("file");
			GMBuffer buffer;
			package->readFile(GMPackageIndex::Audio, file, &buffer);
			IAudioFile* f = nullptr;
			m_audioReader->load(buffer, &f);
			if (!f)
			{
				gm_warning(gm_dbg_wrap("Cannot read audio file {0}."), file);
			}
			else
			{
				m_audioFiles[id] = AutoReleasePtr<IAudioFile>(f);
			}
		}
		else if (name == L"particles")
		{
			parseParticlesAsset(e);
		}

		e = e->NextSiblingElement();
	}
}

void Timeline::parseObjects(GMXMLElement* e)
{
	while (e)
	{
		GMString id = e->Attribute("id");
		if (id.isEmpty())
			gm_warning(gm_dbg_wrap("Each object must have an ID"));

		GMString name = e->Name();
		if (name == L"light")
		{
			GMString typeStr = e->Attribute("type");
			GMLightType type = GMLightType::PointLight;
			if (typeStr == L"point")
				type = GMLightType::PointLight;
			else if (typeStr == L"directional")
				type = GMLightType::DirectionalLight;
			else if (typeStr == L"spotlight")
				type = GMLightType::Spotlight;
			else
				gm_warning(gm_dbg_wrap("Wrong light type: {0}"), typeStr);

			ILight* light = nullptr;
			GM.getFactory()->createLight(type, &light);

			GM_ASSERT(light);
			GMString posStr = e->Attribute("position");
			if (!posStr.isEmpty())
			{
				Scanner scanner(posStr, *this);
				GMfloat x, y, z;
				scanner.nextFloat(x);
				scanner.nextFloat(y);
				scanner.nextFloat(z);
				GMfloat value[] = { x, y, z };
				light->setLightAttribute3(GMLight::Position, value);
			}

			GMString ambientStr = e->Attribute("ambient");
			if (!ambientStr.isEmpty())
			{
				Scanner scanner(ambientStr, *this);
				GMfloat x, y, z;
				scanner.nextFloat(x);
				scanner.nextFloat(y);
				scanner.nextFloat(z);
				GMfloat value[] = { x, y, z };
				light->setLightAttribute3(GMLight::AmbientIntensity, value);
			}

			GMString diffuseStr = e->Attribute("diffuse");
			if (!diffuseStr.isEmpty())
			{
				Scanner scanner(diffuseStr, *this);
				GMfloat x, y, z;
				scanner.nextFloat(x);
				scanner.nextFloat(y);
				scanner.nextFloat(z);
				GMfloat value[] = { x, y, z };
				light->setLightAttribute3(GMLight::DiffuseIntensity, value);
			}

			GMString specularStr = e->Attribute("specular");
			if (!specularStr.isEmpty())
			{
				GMfloat specular = 0;
				Scanner scanner(specularStr, *this);
				scanner.nextFloat(specular);
				bool bSuc = light->setLightAttribute(GMLight::SpecularIntensity, specular);
				GM_ASSERT(bSuc);
			}

			GMString cutOffStr = e->Attribute("cutoff");
			if (!cutOffStr.isEmpty())
			{
				GMfloat cutOff = 0;
				Scanner scanner(cutOffStr, *this);
				scanner.nextFloat(cutOff);
				bool bSuc = light->setLightAttribute(GMLight::CutOff, cutOff);
				GM_ASSERT(bSuc);
			}

			m_lights[id] = AutoReleasePtr<ILight>(light);
		}
		else if (name == L"cubemap")
		{
			GMGameObject* obj = nullptr;
			GMString assetName = e->Attribute("asset");
			GMAsset asset = findAsset(assetName);
			if (!asset.isEmpty())
			{
				obj = new GMCubeMapGameObject(asset);
				m_objects[id] = AutoReleasePtr<GMGameObject>(obj);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot find asset: {0}"), assetName);
			}

			parseTransform(obj, e);
		}
		else if (name == L"object")
		{
			if (objectExists(id))
				return;

			GMGameObject* obj = nullptr;
			GMString assetName = e->Attribute("asset");
			GMAsset asset = findAsset(assetName);
			if (!asset.isEmpty())
			{
				obj = new GMGameObject(asset);
				m_objects[id] = AutoReleasePtr<GMGameObject>(obj);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot find asset: {0}"), assetName);
			}

			parseTextures(obj, e);
			parseMaterial(obj, e);
			parseTransform(obj, e);
			parseBlend(obj, e);
		}
		else if (name == L"bsp")
		{
			if (objectExists(id))
				return;

			BSPGameObject* bsp = nullptr;
			GMString assetName = e->Attribute("asset");
			GMBuffer asset = findBuffer(assetName);
			if (asset.getSize() > 0)
			{
				bsp = new BSPGameObject(m_context);
				bsp->load(asset);
				m_objects[id] = AutoReleasePtr<GMGameObject>(bsp);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Invalid asset buffer: {0}"), assetName);
			}
		}
		else if (name == L"quad")
		{
			if (objectExists(id))
				return;

			GMString lengthStr = e->Attribute("length");
			GMString widthStr = e->Attribute("width");
			GMfloat length = 0, width = 0;
			length = parseFloat(lengthStr);
			width = parseFloat(widthStr);
			
			GMSceneAsset scene;
			GMPrimitiveCreator::createQuadrangle(GMVec2(length / 2, width / 2), 0, scene);

			GMGameObject* obj = new GMGameObject(scene);
			GM_ASSERT(obj->getModel());
			obj->getModel()->getShader().setCull(GMS_Cull::None);
			m_objects[id] = AutoReleasePtr<GMGameObject>(obj);

			GMString type = e->Attribute("type");
			if (!type.isEmpty())
			{
				if (type == "2d")
					obj->getModel()->setType(GMModelType::Model2D);
				else
					gm_warning(gm_dbg_wrap("Cannot recognize type '{0}' of object '{1}"), type, id);
			}

			GMString colorStr = e->Attribute("color");
			if (!colorStr.isEmpty())
			{
				Scanner scanner(colorStr, *this);
				Array<GMfloat, 4> color;
				scanner.nextFloat(color[0]);
				scanner.nextFloat(color[1]);
				scanner.nextFloat(color[2]);
				scanner.nextFloat(color[3]);

				obj->foreachModel([this, color](GMModel* model) {
					model->setUsageHint(GMUsageHint::DynamicDraw);

					// 保存副本
					GMVertices vertices;
					vertices.reserve(model->getVerticesCount());
					for (auto& part : model->getParts())
					{
						for (auto& vertex : part->vertices())
						{
							vertices.push_back(vertex);
						}
					}
					m_verticesCache[model] = std::move(vertices);
					setColorForModel(model, &color[0]);
				});
			}

			GMString colorOpStr = e->Attribute("colorOp");
			GMS_VertexColorOp op = GMS_VertexColorOp::DoNotUseVertexColor;
			if (colorOpStr == "replace")
				op = GMS_VertexColorOp::Replace;
			else if (colorOpStr == "add")
				op = GMS_VertexColorOp::Add;
			else if (colorOpStr == "multiply")
				op = GMS_VertexColorOp::Multiply;
			if (op != GMS_VertexColorOp::DoNotUseVertexColor)
			{
				obj->foreachModel([op](GMModel* model) {
					model->getShader().setVertexColorOp(op);
				});
			}

			parseTextures(obj, e);
			parseMaterial(obj, e);
			parseTransform(obj, e);
			parseBlend(obj, e);
		}
		else if (name == L"wave")
		{
			if (objectExists(id))
				return;

			GMWaveGameObjectDescription objDesc;
			parseWaveObjectAttributes(objDesc, e);

			Vector<GMWaveDescription> wds;
			GMXMLElement* waveDescriptionPtr = e->FirstChildElement("attribute");
			while (waveDescriptionPtr)
			{
				GMWaveDescription desc;
				parseWaveAttributes(desc, waveDescriptionPtr);
				wds.push_back(std::move(desc));
				waveDescriptionPtr = waveDescriptionPtr->NextSiblingElement("attribute");
			}

			GMWaveGameObject* obj = GMWaveGameObject::create(objDesc);
			obj->setWaveDescriptions(std::move(wds));
			m_objects[id] = AutoReleasePtr<GMGameObject>(obj);
			parseTextures(obj, e);
			parseMaterial(obj, e);
			parseTransform(obj, e);
			parseBlend(obj, e);
		}
		else if (name == L"terrain")
		{
			if (objectExists(id))
				return;

			GMfloat terrainX = 0, terrainZ = 0;
			GMfloat width = 1, height = 1, heightScaling = 10.f;

			GMint32 sliceX = 10, sliceY = 10;
			GMfloat texLen = 10, texHeight = 10;

			terrainX = parseFloat(e->Attribute("terrainX"));
			terrainZ = parseFloat(e->Attribute("terrainZ"));
			width = parseFloat(e->Attribute("width"));
			height = parseFloat(e->Attribute("height"));
			heightScaling = parseFloat(e->Attribute("heightScaling"));

			GMString sliceStr = e->Attribute("slice");
			if (!sliceStr.isEmpty())
			{
				Scanner scanner(sliceStr, *this);
				scanner.nextInt(sliceX);
				scanner.nextInt(sliceY);
			}

			GMString texSizeStr = e->Attribute("textureSize");
			if (!texSizeStr.isEmpty())
			{
				Scanner scanner(texSizeStr, *this);
				scanner.nextFloat(texLen);
				scanner.nextFloat(texHeight);
			}

			GMString terrainName = e->Attribute("terrain");
			GMBuffer terrainBuffer = findBuffer(terrainName);
			GMImage* imgMap = nullptr;
			GMSceneAsset scene;
			if (GMImageReader::load(terrainBuffer.getData(), terrainBuffer.getSize(), &imgMap))
			{
				GMTerrainDescription desc = {
					imgMap->getData().mip[0].data,
					imgMap->getData().channels,
					imgMap->getWidth(),
					imgMap->getHeight(),
					terrainX,
					terrainZ,
					width,
					height,
					heightScaling,
					(GMsize_t)sliceX,
					(GMsize_t)sliceY,
					texLen,
					texHeight,
					false
				};

				GMPrimitiveCreator::createTerrain(desc, scene);
				imgMap->destroy();
			}
			else
			{
				GMTerrainDescription desc = {
					nullptr,
					0,
					0,
					0,
					terrainX,
					terrainZ,
					width,
					height,
					heightScaling,
					(GMsize_t)sliceX,
					(GMsize_t)sliceY,
					texLen,
					texHeight,
					false
				};

				GMPrimitiveCreator::createTerrain(desc, scene);
			}

			GMGameObject* obj = new GMGameObject(scene);
			m_objects[id] = AutoReleasePtr<GMGameObject>(obj);
			parseTextures(obj, e);
			parseMaterial(obj, e);
			parseTransform(obj, e);
			parseBlend(obj, e);
		}
		else if (name == L"shadow")
		{
			GMRect rc = m_context->getWindow()->getRenderRect();
			GMfloat width = rc.width, height = rc.height;
			{
				GMString str = e->Attribute("width");
				if (!str.isEmpty())
					width = parseFloat(str);
			}
			{
				GMString str = e->Attribute("height");
				if (!str.isEmpty())
					height = parseFloat(str);
			}

			GMfloat cascades = 1;
			{
				GMString str = e->Attribute("cascades");
				if (!str.isEmpty())
					cascades = parseFloat(str);
			}

			GMfloat bias = .005f;
			{
				GMString str = e->Attribute("bias");
				if (!str.isEmpty())
					bias = parseFloat(str);
			}

			Nullable<GMfloat> pcf;
			{
				GMString str = e->Attribute("pcf");
				if (!str.isEmpty())
					pcf = parseInt(str);
			}

			Vector<GMfloat> partitions;
			if (cascades > 1)
			{
				partitions.resize(cascades);
				GMString str = e->Attribute("partitions");
				if (!str.isEmpty())
				{
					Scanner scanner(str, *this);
					for (GMint32 i = 0; i < cascades; ++i)
					{
						GMfloat p = 0;
						scanner.nextFloat(p);
						partitions[i] = p;
					}
				}
				else
				{
					GMfloat p = 1.f / cascades;
					for (GMint32 i = 0; i < cascades - 1; ++i)
					{
						partitions[i] = (i + 1) * p;
					}
					partitions[cascades - 1] = 1.f;
				}
			}

			CameraParams cp;
			GMCameraLookAt lookAt;
			GMint32 component = parseCameraAction(e, cp, lookAt);
			GMShadowSourceDesc desc;
			desc.type = GMShadowSourceDesc::CSMShadow;
			GMCamera camera;
			if (component & PerspectiveComponent)
				camera.setPerspective(cp.fovy, cp.aspect, cp.n, cp.f);
			else if (component & OrthoComponent)
				camera.setOrtho(cp.left, cp.right, cp.bottom, cp.top, cp.n, cp.f);
			if (component & CameraLookAtComponent)
				camera.lookAt(lookAt);

			desc.position = GMVec4(lookAt.position, 1);
			desc.camera = camera;
			if (cascades > 1)
			{
				for (GMint32 i = 0; i < cascades; ++i)
				{
					desc.cascadePartitions[i] = partitions[i];
				}
			}

			desc.width = width;
			desc.height = height;
			desc.cascades = cascades;
			desc.biasMax = desc.biasMin = bias;
			if (!pcf.isNull)
				desc.pcfRowCount = pcf;
			m_shadows[id] = desc;
		}
		else if (name == L"source")
		{
			GMString assetName = e->Attribute("asset");
			auto asset = m_audioFiles[assetName].get();
			if (asset)
			{
				auto exists = m_audioSources.find(id);
				if (exists == m_audioSources.end())
				{
					IAudioSource* s = nullptr;
					m_audioPlayer->createPlayerSource(asset, &s);
					if (s)
						m_audioSources[id] = AutoReleasePtr<IAudioSource>(s);
					else
						gm_warning(gm_dbg_wrap("Create source failed: {0}"), id);
				}
				else
				{
					gm_warning(gm_dbg_wrap("Source id {0} is already existed."), id);
				}
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot find asset for source: {0}"), assetName);
			}
		}
		else if (name == L"particles")
		{
			parseParticlesObject(e);
		}
		e = e->NextSiblingElement();
	}
}

void Timeline::interpolateCamera(GMXMLElement* e, GMfloat timePoint)
{
	GMint32 component = NoCameraComponent;
	GMVec3 pos, dir, focus;
	GMString str = e->Attribute("position");
	if (!str.isEmpty())
	{
		Scanner scanner(str, *this);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		pos = GMVec3(x, y, z);
		component |= PositionComponent;
	}

	str = e->Attribute("direction");
	if (!str.isEmpty())
	{
		Scanner scanner(str, *this);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		dir = Normalize(GMVec3(x, y, z));
		component |= DirectionComponent;
	}

	str = e->Attribute("focus");
	if (!str.isEmpty())
	{
		Scanner scanner(str, *this);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		focus = GMVec3(x, y, z);
		component |= FocusAtComponent;
	}

	if ((component & DirectionComponent) && (component & FocusAtComponent))
	{
		gm_warning(gm_dbg_wrap("You cannot specify both 'direction' and 'focus'. In this camera action, 'direction' won't work."));
	}
	if (!(component & DirectionComponent) && !(component & FocusAtComponent))
	{
		gm_warning(gm_dbg_wrap("You must specify at least 'direction' or 'focus'. In this camera action, 'direction' won't work."));
	}

	if (component != NoCameraComponent)
	{
		GMInterpolationFunctors f;
		CurveType curve = parseCurve(e, f);
		GMCamera& camera = m_context->getEngine()->getCamera();
		AnimationContainer& animationContainer = m_animations[Camera][&camera];

		GMAnimation& animation = animationContainer.currentEditingAnimation();
		const GMCameraLookAt& lookAt = camera.getLookAt();
		animation.setTargetObjects(&camera);
			
		GMVec3 posCandidate = (component & PositionComponent) ? pos : lookAt.position;
		GMVec3 dirCandidate = (component & DirectionComponent) ? dir : lookAt.lookDirection;
		GMVec3 focusCandidate = (component & FocusAtComponent) ? focus : lookAt.position + lookAt.lookDirection;
		GMCameraKeyframe* keyframe = nullptr;
		if (component & DirectionComponent)
			keyframe = new GMCameraKeyframe(GMCameraKeyframeComponent::LookAtDirection, posCandidate, dirCandidate, timePoint);
		else // 默认是按照focusAt调整视觉
			keyframe = new GMCameraKeyframe(GMCameraKeyframeComponent::FocusAt, posCandidate, focusCandidate, timePoint);
		if (curve != CurveType::NoCurve)
			keyframe->setFunctors(f);
		animation.addKeyFrame(keyframe);
	}
}

void Timeline::interpolateLight(GMXMLElement* e, ILight* light, GMfloat timePoint)
{
	GMint32 component = GMLightKeyframeComponent::NoComponent;
	GMVec3 position, ambient, diffuse;
	GMfloat specular = 0, cutOff = 10.f;

	GMString ambientStr = e->Attribute("ambient");
	if (!ambientStr.isEmpty())
	{
		Scanner scanner(ambientStr, *this);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		ambient = GMVec3(x, y, z);
		component |= GMLightKeyframeComponent::Ambient;
	}

	GMString diffuseStr = e->Attribute("diffuse");
	if (!diffuseStr.isEmpty())
	{
		Scanner scanner(diffuseStr, *this);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		diffuse = GMVec3(x, y, z);
		component |= GMLightKeyframeComponent::Diffuse;
	}

	GMString specularStr = e->Attribute("specular");
	if (!specularStr.isEmpty())
	{
		Scanner scanner(specularStr, *this);
		scanner.nextFloat(specular);
		component |= GMLightKeyframeComponent::Specular;
	}

	GMString cutOffStr = e->Attribute("cutoff");
	if (!cutOffStr.isEmpty())
	{
		Scanner scanner(specularStr, *this);
		scanner.nextFloat(cutOff);
		component |= GMLightKeyframeComponent::CutOff;
	}

	GMString posStr = e->Attribute("position");
	if (!posStr.isEmpty())
	{
		Scanner scanner(posStr, *this);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		position = GMVec3(x, y, z);
		component |= GMLightKeyframeComponent::Position;
	}

	if (component != GMLightKeyframeComponent::NoComponent)
	{
		GMInterpolationFunctors f;
		CurveType curve = parseCurve(e, f);
		AnimationContainer& animationContainer = m_animations[Light][light];
		GMAnimation& animation = animationContainer.currentEditingAnimation();

		animation.setTargetObjects(light);
		GMAnimationKeyframe* keyframe = new GMLightKeyframe(m_context, component, ambient, diffuse, specular, position, cutOff, timePoint);
		if (curve != CurveType::NoCurve)
			keyframe->setFunctors(f);
		animation.addKeyFrame(keyframe);
	}
}

void Timeline::interpolateObject(GMXMLElement* e, GMGameObject* obj, GMfloat timePoint)
{
	GMint32 component = GMGameObjectKeyframeComponent::NoComponent;
	GMVec4 translation, scaling;
	GMQuat rotation;

	GMString str = e->Attribute("translate");
	if (!str.isEmpty())
	{
		Scanner scanner(str, *this);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		component |= GMGameObjectKeyframeComponent::Translate;
		translation = GMVec4(x, y, z, 1);
	}

	str = e->Attribute("scale");
	if (!str.isEmpty())
	{
		Scanner scanner(str, *this);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		component |= GMGameObjectKeyframeComponent::Scale;
		scaling = GMVec4(x, y, z, 1);
	}

	str = e->Attribute("rotate");
	if (!str.isEmpty())
	{
		GMfloat x, y, z, degree;
		Scanner scanner(str, *this);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		scanner.nextFloat(degree);
		component |= GMGameObjectKeyframeComponent::Rotate;
		GMVec3 axis(x, y, z);
		if (!FuzzyCompare(Length(axis), 0.f))
			rotation = Rotate(Radian(degree), axis);
		else
			gm_warning(gm_dbg_wrap("Wrong rotation axis"));
	}

	if (component != GMGameObjectKeyframeComponent::NoComponent)
	{
		GMInterpolationFunctors f;
		CurveType curve = parseCurve(e, f);
		AnimationContainer& animationContainer = m_animations[GameObject][obj];
		GMAnimation& animation = animationContainer.currentEditingAnimation();
		animation.setTargetObjects(obj);
		GMAnimationKeyframe* keyframe = new GMGameObjectKeyframe(component, translation, scaling, rotation, timePoint);
		if (curve != CurveType::NoCurve)
			keyframe->setFunctors(f);
		animation.addKeyFrame(keyframe);
	}

	str = e->Attribute("color");
	if (!str.isEmpty())
	{
		GMInterpolationFunctors f;
		CurveType curve = parseCurve(e, f);
		AnimationContainer& animationContainer = m_animations[GameObject][obj];
		GMAnimation& animation = animationContainer.currentEditingAnimation();
		animation.setTargetObjects(obj);

		Scanner scanner(str, *this);
		Array<float, 4> color;
		scanner.nextFloat(color[0]);
		scanner.nextFloat(color[1]);
		scanner.nextFloat(color[2]);
		scanner.nextFloat(color[3]);

		GMAnimationKeyframe* keyframe = new GameObjectColorKeyframe(m_verticesCache, color, timePoint);
		if (curve != CurveType::NoCurve)
			keyframe->setFunctors(f);
		animation.addKeyFrame(keyframe);
	}
}

AnimationContainer& Timeline::getAnimationFromObject(AssetType at, void* o)
{
	if (at == AssetType::NotFound)
		throw std::logic_error("Asset not found");

	if (at == AssetType::Light)
		return m_animations[Light][asset_cast<ILight>(o)->get()];
	if (at == AssetType::GameObject)
		return m_animations[GameObject][asset_cast<GMGameObject>(o)->get()];
	if (at == AssetType::Camera)
		return m_animations[Camera][asset_cast<GMCamera>(o)];

	throw std::logic_error("Asset wrong type.");
}

void Timeline::parseActions(GMXMLElement* e)
{
	GM_ASSERT(m_world);
	while (e)
	{
		GMString name = e->Name();
		if (name == L"include")
		{
			parseInclude(e);
		}
		else if (name == L"component")
		{
			parseComponent(e);
		}
		else if (name == L"action")
		{
			Action action = { Action::Immediate };
			GMString time = e->Attribute("time");
			if (!time.isEmpty())
			{
				action.runType = Action::Deferred;
				if (time[0] == '+' || time[0] == '-')
				{
					// 相对时间
					if (!time.isEmpty())
					{
						action.timePoint = m_lastTime + parseFloat(time);
						if (action.timePoint > m_lastTime)
							m_lastTime = action.timePoint;
					}
					else
					{
						action.runType = Action::Immediate;
						gm_warning(gm_dbg_wrap("Wrong action time: {0}"), time);
					}
				}
				else
				{
					action.timePoint = parseFloat(time);
					if (action.timePoint > m_lastTime)
						m_lastTime = action.timePoint;
				}
			}

			if (action.timePoint == 0)
				action.runType = Action::Immediate;

			GMString type = e->Attribute("type");
			if (type == L"camera")
			{
				CameraParams p;
				GMCameraLookAt lookAt;
				GMint32 component = parseCameraAction(e, p, lookAt);
				GMsize_t lookAtIndex = m_cameraLookAtCache.put(lookAt);
				
				if (component & PerspectiveComponent)
				{
					action.action = [p, this]() {
						auto& camera = m_context->getEngine()->getCamera();
						camera.setPerspective(p.fovy, p.aspect, p.n, p.f);
					};
				}
				else if (component & OrthoComponent)
				{
					action.action = [p, this]() {
						auto& camera = m_context->getEngine()->getCamera();
						camera.setOrtho(p.left, p.right, p.bottom, p.top, p.n, p.f);
					};
				}
				if (component & CameraLookAtComponent)
				{
					action.action = [p, action, lookAtIndex, this]() {
						if (action.action)
							action.action();

						auto& camera = m_context->getEngine()->getCamera();
						camera.lookAt(m_cameraLookAtCache[lookAtIndex]);
					};
				}

				bindAction(action);
			}
			else if (type == L"addObject")
			{
				GMString object = e->Attribute("object");
				void* targetObject = nullptr;
				AssetType assetType = getAssetType(object, &targetObject);

				if (assetType == AssetType::GameObject)
					addObject(asset_cast<GMGameObject>(targetObject), e, action);
				else if (assetType == AssetType::Light)
					addObject(asset_cast<ILight>(targetObject), e, action);
				else if (assetType == AssetType::Particles)
					addObject(asset_cast<IParticleSystem>(targetObject), e, action);
				else if (assetType == AssetType::Shadow)
					addObject(asset_cast<GMShadowSourceDesc>(targetObject), e, action);
				else
					gm_warning(gm_dbg_wrap("Cannot find object: {0}"), object);

				bindAction(action);
			}
			else if (type == L"animate") // 或者是其它插值变换
			{
				GMString actionStr = e->Attribute("action");
				GMString object = e->Attribute("object");
				void* targetObject = nullptr;
				AssetType assetType = getAssetType(object, &targetObject);

				if (assetType == AssetType::NotFound)
					gm_warning(gm_dbg_wrap("Cannot find object: {0}"), object);

				if (actionStr == L"play")
				{
					if (targetObject)
					{
						AnimationContainer& animationContainer = getAnimationFromObject(assetType, targetObject);
						animationContainer.nextEditAnimation();

						GMfloat timePoint = action.timePoint;
						action.action = [&animationContainer, timePoint]() {
							animationContainer.playAnimation();
						};
						bindAction(action);
					}
				}
				else if (actionStr == L"stop")
				{
					if (targetObject)
					{
						AnimationContainer& animationContainer = getAnimationFromObject(assetType, targetObject);
						action.action = [&animationContainer]() {
							animationContainer.pauseAnimation();
							animationContainer.nextPlayingAnimation();
						};
						bindAction(action);
					}
				}
				else
				{
					GMString endTimeStr = e->Attribute("endtime");
					GMfloat endTime = 0;
					bool ok = false;
					endTime = parseFloat(endTimeStr, &ok);
					if (ok)
					{
						action.timePoint = 0;
						action.runType = Action::Immediate; // 插值动作的添加是立即的

						if (assetType == AssetType::Camera)
							interpolateCamera(e, endTime);
						else if (assetType == AssetType::Light)
							interpolateLight(e, asset_cast<ILight>(targetObject)->get(), endTime);
						else if (assetType == AssetType::GameObject)
							interpolateObject(e, asset_cast<GMGameObject>(targetObject)->get(), endTime);
					}
					else
					{
						gm_warning(gm_dbg_wrap("Attribute 'endtime' must be specified in animation interpolation. Object: '{0}'"), object);
					}
				}
			}
			else if (type == L"attribute")
			{
				GMString object = e->Attribute("object");
				void* targetObject = nullptr;
				AssetType assetType = getAssetType(object, &targetObject);
				if (assetType == AssetType::GameObject)
					parseAttributes(asset_cast<GMGameObject>(targetObject)->get(), e, action);
				else if (assetType == AssetType::NotFound)
					gm_warning(gm_dbg_wrap("Object '{0}' not found."), object);
				else
					gm_warning(gm_dbg_wrap("Object '{0}' doesn't support 'attribute' type."), object);
				bindAction(action);
			}
			else if (type == L"removeObject")
			{
				GMString object = e->Attribute("object");
				void* targetObject = nullptr;
				AssetType assetType = getAssetType(object, &targetObject);

				GMString deleteFromMemoryStr = e->Attribute("delete");
				bool deleteFromMemory = deleteFromMemoryStr.isEmpty() ? false : toBool(deleteFromMemoryStr);
				if (assetType == AssetType::Light)
					removeObject(asset_cast<ILight>(targetObject)->get(), e, action);
				else if (assetType == AssetType::GameObject)
					removeObject(asset_cast<GMGameObject>(targetObject)->get(), e, action, deleteFromMemory);
				else if (assetType == AssetType::NotFound)
					gm_warning(gm_dbg_wrap("Object '{0}' not found."), object);
				else
					gm_warning(gm_dbg_wrap("Object '{0}' doesn't support 'removeObject' type."), object);
				bindAction(action);
			}
			else if (type == L"removeShadow")
			{
				action.action = [this]() {
					GMShadowSourceDesc desc;
					desc.type = GMShadowSourceDesc::NoShadow;
					m_context->getEngine()->setShadowSource(desc);
				};
				bindAction(action);
			}
			else if (type == L"play")
			{
				GMString object = e->Attribute("object");
				void* targetObject = nullptr;
				AssetType assetType = getAssetType(object, &targetObject);
				if (assetType == AssetType::AudioSource)
				{
					action.action = [this, targetObject]() {
						playAudio(asset_cast<IAudioSource>(targetObject)->get());
					};
					bindAction(action);
				}
				else if (assetType == AssetType::GameObject)
				{
					GMString n = e->Attribute("name");
					action.action = [this, targetObject, n](){
						play(asset_cast<GMGameObject>(targetObject)->get(), n);
					};
					bindAction(action);
				}
				else
				{
					gm_warning(gm_dbg_wrap("Cannot find source: {0}"), object);
				}
			}
			else if (type == L"emit")
			{
				GMString object = e->Attribute("object");
				void* targetObject = nullptr;
				AssetType assetType = getAssetType(object, &targetObject);
				if (assetType == AssetType::Particles)
				{
					auto particleSystemPtr = asset_cast<IParticleSystem>(targetObject);
					GM_ASSERT(particleSystemPtr);
					IParticleSystem* particleSystem = particleSystemPtr->get();
					action.action = [particleSystem](){
						particleSystem->getEmitter()->emitOnce();
						particleSystem->getEmitter()->startEmit();
					};
					bindAction(action);
				}
				else
				{
					gm_warning(gm_dbg_wrap("object '{0}' is not a particle system object."), object);
				}
			}
			else if (type == L"filter")
			{
				GMString name = e->Attribute("name");
				if (name == L"blend")
				{
					GMString valueStr = e->Attribute("value");
					GMfloat r = 1, g = 1, b = 1;
					Scanner scanner(valueStr, *this);
					scanner.nextFloat(r);
					scanner.nextFloat(g);
					scanner.nextFloat(b);

					action.action = [this, r, g, b]() {
						auto renderContext = m_context->getEngine()->getConfigs().getConfig(GMConfigs::Render).asRenderConfig();
						renderContext.set(GMRenderConfigs::FilterMode, GMFilterMode::Blend);
						renderContext.set(GMRenderConfigs::BlendFactor_Vec3, GMVec3(r, g, b));
					};
				}
				bindAction(action);
			}
			else if (type == L"removeFilter")
			{
				action.action = [this]() {
					auto renderContext = m_context->getEngine()->getConfigs().getConfig(GMConfigs::Render).asRenderConfig();
					renderContext.set(GMRenderConfigs::FilterMode, GMFilterMode::None);
				};
				bindAction(action);
			}
			else
			{
				gm_warning(gm_dbg_wrap("action type cannot be recognized: {0}"), type);
			}
		}
		else if (name == L"checkpoint")
		{
			GMString type = e->Attribute("type");
			if (type == L"save")
			{
				m_checkpointTime = m_lastTime;
			}
			else if (type == L"load")
			{
				m_lastTime = m_checkpointTime;
			}
			else if (type == L"time")
			{
				GMString timeStr = e->Attribute("time");
				if (!timeStr.isEmpty())
				{
					GMfloat time = 0;
					bool ok = false;
					time = parseFloat(timeStr, &ok);
					if (ok)
					{
						m_lastTime += time;
						m_checkpointTime = m_lastTime;
					}
					else
					{
						gm_warning(gm_dbg_wrap("'time' is not a number: {0}"), timeStr);
					}
				}
				else
				{
					gm_warning(gm_dbg_wrap("checkpoint 'time' type missing 'time' attribute."));
				}
			}
			else
			{
				gm_warning(gm_dbg_wrap("checkpoint type cannot be recognized: {0}"), type);
			}
		}
		else
		{
			gm_warning(gm_dbg_wrap("tag name cannot be recognized: {0}"), e->Name());
		}

		e = e->NextSiblingElement();
	}
}

void Timeline::parseInclude(GMXMLElement* e)
{
	GMString filename = e->Attribute("file");
	if (filename.isEmpty())
	{
		gm_warning(gm_dbg_wrap("Include file name cannot be empty."));
	}
	else
	{
		GMBuffer buffer;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Scripts, filename, &buffer);
		if (buffer.getSize() > 0)
		{
			buffer.convertToStringBuffer();
			GMString content((const char*)buffer.getData());
			parse(content);
		}
		else
		{
			gm_warning(gm_dbg_wrap("File is empty with filename: {0}"), filename);
		}
	}
}

void Timeline::parseComponent(GMXMLElement* e)
{
	// component和include大体一致。只不过component的对象只会添加一次，并且可以接受槽作为参数。
	GMString filename = e->Attribute("file");
	if (filename.isEmpty())
	{
		gm_warning(gm_dbg_wrap("Component file name cannot be empty."));
	}
	else
	{
		// 解析Component下所有槽，再解析内容
		m_supressedWarnings.set(Warning_ObjectExistsWarning);
		m_slots.pushSlots();
		{
			GMXMLElement* el = e->FirstChildElement();
			while (el)
			{
				if (GMString::stringEquals(el->Name(), "slot"))
					m_slots.addSlot(el->Attribute("name"), getValueFromDefines(el->GetText()));
				el = el->NextSiblingElement();
			}
		}

		GMBuffer buffer;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Scripts, filename, &buffer);
		if (buffer.getSize() > 0)
		{
			buffer.convertToStringBuffer();
			GMString content((const char*)buffer.getData());
			parseComponent(content);
		}
		else
		{
			gm_warning(gm_dbg_wrap("File is empty with filename: {0}"), filename);
		}
		m_slots.popSlots();
		m_supressedWarnings.clear(Warning_ObjectExistsWarning);
	}
}

GMint32 Timeline::parseCameraAction(GMXMLElement* e, CameraParams& cp, GMCameraLookAt& lookAt)
{
	GMint32 component = NoCameraComponent;
	GMString view = e->Attribute("view");
	if (view == L"perspective")
	{
		GMString fovyStr, aspectStr, nStr, fStr;
		GMfloat fovy, aspect, n, f;
		fovyStr = e->Attribute("fovy");
		if (fovyStr.isEmpty())
			fovy = Radian(75.f);
		else
			fovy = Radian(parseFloat(fovyStr));

		aspectStr = e->Attribute("aspect");
		if (aspectStr.isEmpty())
		{
			GMRect rc = m_context->getWindow()->getRenderRect();
			aspect = static_cast<GMfloat>(rc.width) / rc.height;
		}
		else
		{
			aspect = 1.33f;
		}

		nStr = e->Attribute("near");
		if (nStr.isEmpty())
			n = .1f;
		else
			n = parseFloat(nStr);

		fStr = e->Attribute("far");
		if (fStr.isEmpty())
			f = 3200;
		else
			f = parseFloat(fStr);

		cp.fovy = fovy;
		cp.aspect = aspect;
		cp.n = n;
		cp.f = f;
		component |= PerspectiveComponent;
	}
	else if (view == L"ortho")
	{
		GMfloat left = 0, right = 0, top = 0, bottom = 0;
		GMfloat n = 0, f = 0;
		left = parseInt(e->Attribute("left"));
		right = parseInt(e->Attribute("right"));
		top = parseInt(e->Attribute("top"));
		bottom = parseInt(e->Attribute("bottom"));
		n = parseInt(e->Attribute("near"));
		f = parseInt(e->Attribute("far"));
		cp.left = left;
		cp.right = right;
		cp.top = top;
		cp.bottom = bottom;
		cp.n = n;
		cp.f = f;
		component |= OrthoComponent;
	}
	else if (!view.isEmpty())
	{
		gm_warning(gm_dbg_wrap("Camera view only supports 'perspective' and 'ortho'"));
	}

	// 设置位置
	GMString dirStr, posStr, focusStr;
	GMVec3 direction = Zero<GMVec3>(), position = Zero<GMVec3>(), focus = Zero<GMVec3>();
	dirStr = e->Attribute("direction");
	posStr = e->Attribute("position");
	focusStr = e->Attribute("focus");
	if (!posStr.isEmpty())
	{
		GMfloat x = 0, y = 0, z = 0;
		Scanner scanner(posStr, *this);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		position = GMVec3(x, y, z);
	}

	if (!dirStr.isEmpty())
	{
		GMfloat x = 0, y = 0, z = 0;
		Scanner scanner(dirStr, *this);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		direction = GMVec3(x, y, z);
	}

	if (!focusStr.isEmpty())
	{
		GMfloat x = 0, y = 0, z = 0;
		Scanner scanner(focusStr, *this);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		focus = GMVec3(x, y, z);
	}

	if (!dirStr.isEmpty() && !focusStr.isEmpty())
	{
		gm_warning(gm_dbg_wrap("You cannot specify both 'direction' and 'focus'. 'direction' won't work."));
	}

	if (!focusStr.isEmpty())
	{
		// focus
		lookAt = GMCameraLookAt::makeLookAt(position, focus);
		component |= CameraLookAtComponent;
	}
	else if (!dirStr.isEmpty())
	{
		// direction
		lookAt = GMCameraLookAt(direction, position);
		component |= CameraLookAtComponent;
	}
	return component;
}

void Timeline::parseParticlesAsset(GMXMLElement* e)
{
	GMString id = e->Attribute("id");
	GMString filename = e->Attribute("file");
	if (!id.isEmpty())
	{
		GMBuffer& buf = m_buffers[id];
		GM.getGamePackageManager()->readFile(GMPackageIndex::Particle, filename, &buf);
		buf.convertToStringBuffer();
	}
	else
	{
		gm_warning(gm_dbg_wrap("ID is empty while parsing particles asset."));
	}
}

void Timeline::parseParticlesObject(GMXMLElement* e)
{
	GMString id = e->Attribute("id");
	GMString assetName = e->Attribute("asset");
	const auto& buf = m_buffers[assetName];
	if (buf.getSize() > 0)
	{
		GMString type = e->Attribute("type");
		if (type == L"cocos2d")
		{
			IParticleSystem* ps = nullptr;
			GMParticleSystem_Cocos2D* cocos2DPs = nullptr;
			GMParticleSystem_Cocos2D::createCocos2DParticleSystem(m_context, buf, GMParticleModelType::Particle3D, &cocos2DPs);
			ps = cocos2DPs;
			if (ps)
			{
				ps->getEmitter()->stopEmit();
				m_particleSystems[id] = AutoReleasePtr<IParticleSystem>(ps);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Create particle system failed for id '{1}'."), id);
			}

			parseCocos2DParticleAttributes(ps, e);
		}
		else
		{
			gm_warning(gm_dbg_wrap("Unrecognized type {0} for id '{1}'."), type, id);
		}
	}
	else
	{
		gm_warning(gm_dbg_wrap("Cannot find particles for source: {0}"), assetName);
	}
}

void Timeline::parseCocos2DParticleAttributes(IParticleSystem* ps, GMXMLElement* e)
{
	GMParticleEmitter_Cocos2D* emitter = gm_cast<GMParticleEmitter_Cocos2D*>(ps->getEmitter());
	GMString translateStr = e->Attribute("translate");
	if (!translateStr.isEmpty())
	{
		GMfloat x = 0, y = 0, z = 0;
		Scanner scanner(translateStr, *this);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		emitter->setEmitPosition(GMVec3(x, y, z));
	}
}

GMAsset Timeline::findAsset(const GMString& assetName)
{
	auto assetIter = m_assets.find(assetName);
	if (assetIter != m_assets.end())
		return assetIter->second;

	return GMAsset::invalidAsset();
}

const PBR* Timeline::findPBR(const GMString& assetName)
{
	auto pbrIter = m_pbrs.find(assetName);
	if (pbrIter != m_pbrs.end())
		return &pbrIter->second;

	return nullptr;
}

GMBuffer Timeline::findBuffer(const GMString& bufferName)
{
	auto bufferIter = m_buffers.find(bufferName);
	if (bufferIter != m_buffers.end())
		return bufferIter->second;

	static GMBuffer s_empty;
	return s_empty;
}

void Timeline::parseTransform(GMGameObject* o, GMXMLElement* e)
{
	if (!o)
		return;

	GMString str = e->Attribute("scale");
	if (!str.isEmpty())
	{
		GMfloat x, y, z;
		Scanner scanner(str, *this);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		o->setScaling(Scale(GMVec3(x, y, z)));
	}

	str = e->Attribute("translate");
	if (!str.isEmpty())
	{
		GMfloat x, y, z;
		Scanner scanner(str, *this);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		o->setTranslation(Translate(GMVec3(x, y, z)));
	}

	str = e->Attribute("rotate");
	if (!str.isEmpty())
	{
		GMfloat x, y, z, degree;
		Scanner scanner(str, *this);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		scanner.nextFloat(degree);
		GMVec3 axis(x, y, z);
		if (!FuzzyCompare(Length(axis), 0.f))
			o->setRotation(Rotate(Radian(degree), axis));
		else
			gm_warning(gm_dbg_wrap("Wrong rotation axis"));
	}
}

void Timeline::parseTextures(GMGameObject* o, GMXMLElement* e)
{
	if (!o)
		return;

	ShaderCallback cbAmbient = parseTexture(e, "ambient", GMTextureType::Ambient);
	ShaderCallback cbDiffuse = parseTexture(e, "diffuse", GMTextureType::Diffuse);
	ShaderCallback cbNormal = parseTexture(e, "normal", GMTextureType::NormalMap);
	ShaderCallback cbSpecular = parseTexture(e, "specular", GMTextureType::Specular);

	ShaderCallback cbAmbientSpeed = parseTextureTransform(e, "ambientTranslateSpeed", GMTextureType::Ambient, GMS_TextureTransformType::Scroll);
	ShaderCallback cbDiffuseSpeed = parseTextureTransform(e, "diffuseTranslateSpeed", GMTextureType::Diffuse, GMS_TextureTransformType::Scroll);
	ShaderCallback cbNormalSpeed = parseTextureTransform(e, "normalTranslateSpeed", GMTextureType::NormalMap, GMS_TextureTransformType::Scroll);
	ShaderCallback cbSpecularSpeed = parseTextureTransform(e, "specularTranslateSpeed", GMTextureType::Specular, GMS_TextureTransformType::Scroll);
	ShaderCallback cbPbr = parseTexturePBR(e);

	GMString id = e->Attribute("id");
	if (isAny(cbAmbient, cbDiffuse, cbNormal, cbSpecular, cbAmbientSpeed, cbDiffuseSpeed, cbNormalSpeed, cbSpecularSpeed, cbPbr))
	{
		o->foreachModel([this, e, cbAmbient, cbDiffuse, cbNormal, cbSpecular, cbAmbientSpeed, cbDiffuseSpeed, cbNormalSpeed, cbSpecularSpeed, cbPbr](GMModel* model)
		{
			if (!model)
				return;

			GMShader& shader = model->getShader();
			callEvery_1(shader, cbAmbient, cbDiffuse, cbNormal, cbSpecular);
			callEvery_1(shader, cbAmbientSpeed, cbDiffuseSpeed, cbNormalSpeed, cbSpecularSpeed);
			callEvery_1(shader, cbPbr);
		});
	}
}

void Timeline::parseMaterial(GMGameObject* o, GMXMLElement* e)
{
	if (!o)
		return;

	ShaderCallback cbMaterial = parseMaterial(e);

	o->foreachModel([this, e, cbMaterial](GMModel* model) {
		GMShader& shader = model->getShader();
		callEvery_1(shader, cbMaterial);
	});
}

Timeline::ShaderCallback Timeline::parseMaterial(GMXMLElement* e)
{
	GMString kaStr = e->Attribute("ka");
	GMString kdStr = e->Attribute("kd");
	GMString ksStr = e->Attribute("ks");
	GMString shininessStr = e->Attribute("shininess");

	struct XYZ { GMfloat x, y, z; };
	Nullable<XYZ> ka, ks, kd;
	Nullable<GMfloat> shininess;
	if (!kaStr.isEmpty())
	{
		XYZ xyz;
		Scanner scanner(kaStr, *this);
		scanner.nextFloat(xyz.x);
		scanner.nextFloat(xyz.y);
		scanner.nextFloat(xyz.z);
		ka = xyz;
	}

	if (!kdStr.isEmpty())
	{
		XYZ xyz;
		Scanner scanner(kdStr, *this);
		scanner.nextFloat(xyz.x);
		scanner.nextFloat(xyz.y);
		scanner.nextFloat(xyz.z);
		kd = xyz;
	}

	if (!ksStr.isEmpty())
	{
		XYZ xyz;
		Scanner scanner(ksStr, *this);
		scanner.nextFloat(xyz.x);
		scanner.nextFloat(xyz.y);
		scanner.nextFloat(xyz.z);
		ks = xyz;
	}

	if (!shininessStr.isEmpty())
		shininess = parseFloat(shininessStr);

	if (isAny(!ka.isNull, !kd.isNull, !ks.isNull, !shininess.isNull))
	{
		ShaderCallback cb = [this, e, ka, kd, ks, shininess](GMShader& shader) {
			if (!ka.isNull)
				shader.getMaterial().setAmbient(GMVec3(ka.value.x, ka.value.y, ka.value.z));
			if (!kd.isNull)
				shader.getMaterial().setDiffuse(GMVec3(kd.value.x, kd.value.y, kd.value.z));
			if (!ks.isNull)
				shader.getMaterial().setSpecular(GMVec3(ks.value.x, ks.value.y, ks.value.z));
			if (!shininess.isNull)
				shader.getMaterial().setShininess(shininess.value);
		};
		return cb;
	}
	return ShaderCallback();
}

void Timeline::parseBlend(GMGameObject* o, GMXMLElement* e)
{
	if (!o)
		return;

	GMString blendStr = e->Attribute("blend");
	Nullable<bool> blend;
	if (!blendStr.isEmpty())
		blend = toBool(blendStr);

	Nullable<GMS_BlendFunc> source, dest;
	GMString blendSrcStr = e->Attribute("source");
	GMString blendDestStr = e->Attribute("dest");
	if (!blendSrcStr.isEmpty())
		source = toBlendFunc(blendSrcStr);
	if (!blendDestStr.isEmpty())
		dest = toBlendFunc(blendDestStr);

	GMString id = e->Attribute("id");
	if (!blend.isNull || !source.isNull || !dest.isNull)
	{
		o->foreachModel([=](GMModel* model)
		{
			GMShader& shader = model->getShader();
			if (!blend.isNull)
				shader.setBlend(blend);

			if (!source.isNull)
				shader.setBlendFactorSource(source);

			if (!dest.isNull)
				shader.setBlendFactorDest(dest);
		});
	}
}

Timeline::ShaderCallback Timeline::parseTexture(GMXMLElement* e, const char* type, GMTextureType textureType)
{
	GMString tex = e->Attribute(type);
	if (!tex.isEmpty())
	{
		return [this, tex, textureType](GMShader& shader) {
			GMAsset asset = findAsset(tex);
			if (!asset.isEmpty() && asset.isTexture())
			{
				GMToolUtil::addTextureToShader(shader, asset, textureType);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot find texture asset: {0}"), tex);
			}
		};
	}
	return ShaderCallback();
}

Timeline::ShaderCallback Timeline::parseTextureTransform(GMXMLElement* e, const char* type, GMTextureType textureType, GMS_TextureTransformType transformType)
{
	GMString val = e->Attribute(type);
	if (!val.isEmpty())
	{
		return [this, val, textureType, transformType](GMShader& shader) {
			Scanner scanner(val, *this);
			GMS_TextureTransform tt;
			tt.type = transformType;
			scanner.nextFloat(tt.p1);
			scanner.nextFloat(tt.p2);
			shader.getTextureList().getTextureSampler(textureType).setTextureTransform(0, tt);
		};
	}
	return ShaderCallback();
}

Timeline::ShaderCallback Timeline::parseTexturePBR(GMXMLElement* e)
{
	GMString pbrStr = e->Attribute("pbr");
	if (!pbrStr.isEmpty())
	{
		ShaderCallback cb = [this, e, pbrStr](GMShader& shader) {
			const PBR* pbr = findPBR(pbrStr);
			if (pbr)
			{
				shader.setIlluminationModel(gm::GMIlluminationModel::CookTorranceBRDF);
				GMToolUtil::addTextureToShader(shader, pbr->albedo, gm::GMTextureType::Albedo);
				GMToolUtil::addTextureToShader(shader, pbr->normal, gm::GMTextureType::NormalMap);
				GMToolUtil::addTextureToShader(shader, pbr->metallicRoughnessAO, gm::GMTextureType::MetallicRoughnessAO);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot find pbr assets: {0}"), pbrStr);
			}
		};
		return cb;
	}
	return ShaderCallback();
}

void Timeline::parseAttributes(GMGameObject* obj, GMXMLElement* e, Action& action)
{
	GM_ASSERT(obj);
	{
		GMString value = e->Attribute("visible");
		if (!value.isEmpty())
		{
			bool visible = toBool(value);
			action.action = [action, obj, visible]() {
				if (action.action)
					action.action();
				obj->setVisible(visible);
			};
		}
	}

	{
		GMString value = e->Attribute("color");
		if (!value.isEmpty())
		{
			Scanner scanner(value, *this);
			Array<GMfloat, 4> color;
			scanner.nextFloat(color[0]);
			scanner.nextFloat(color[1]);
			scanner.nextFloat(color[2]);
			scanner.nextFloat(color[3]);

			action.action = [this, action, obj, color]() {
				obj->foreachModel([action, this, color, obj](GMModel* model) {
					if (action.action)
						action.action();
					transferColorForModel(model, &color[0]);
				});
			};
		}
	}
}

void Timeline::parseWaveObjectAttributes(GMWaveGameObjectDescription& desc, GMXMLElement* e)
{
	GMfloat terrainX = 0, terrainZ = 0;
	GMfloat terrainLength = 1, terrainWidth = 1, heightScaling = 10.f;

	GMint32 sliceX = 10, sliceY = 10;
	GMfloat texLen = 10, texHeight = 10;

	GMfloat texScaleLen = 2, texScaleHeight = 2;

	terrainX = parseFloat(e->Attribute("terrainX"));
	terrainZ = parseFloat(e->Attribute("terrainZ"));
	terrainLength = parseFloat(e->Attribute("length"));
	terrainWidth = parseFloat(e->Attribute("width"));
	heightScaling = parseFloat(e->Attribute("heightScaling"));

	GMString sliceStr = e->Attribute("slice");
	if (!sliceStr.isEmpty())
	{
		Scanner scanner(sliceStr, *this);
		scanner.nextInt(sliceX);
		scanner.nextInt(sliceY);
	}

	GMString texSizeStr = e->Attribute("textureSize");
	if (!texSizeStr.isEmpty())
	{
		Scanner scanner(texSizeStr, *this);
		scanner.nextFloat(texLen);
		scanner.nextFloat(texHeight);
	}

	GMString texScaling = e->Attribute("textureScaling");
	if (!texScaling.isEmpty())
	{
		Scanner scanner(texScaling, *this);
		scanner.nextFloat(texScaleLen);
		scanner.nextFloat(texScaleHeight);
	}

	desc.terrainX = terrainX;
	desc.terrainZ = terrainZ;
	desc.terrainLength = terrainLength;
	desc.terrainWidth = terrainWidth;
	desc.heightScaling = heightScaling;
	desc.sliceM = sliceX;
	desc.sliceN = sliceY;
	desc.textureLength = texLen;
	desc.textureHeight = texHeight;
	desc.textureScaleLength = texScaleLen;
	desc.textureScaleHeight = texScaleHeight;
}

void Timeline::parseWaveAttributes(GMWaveDescription& desc, GMXMLElement* e)
{
	GMfloat steepness = 0;
	GMfloat amplitude = 1;
	GMfloat direction[3];
	GMfloat speed = 2;
	GMfloat waveLength = 1;

	GMString steepnessStr = e->Attribute("steepness");
	if (!steepnessStr.isEmpty())
		steepness = parseFloat(steepnessStr);

	GMString amplitudeStr = e->Attribute("amplitude");
	if (!amplitudeStr.isEmpty())
		amplitude = parseFloat(amplitudeStr);

	GMString directionStr = e->Attribute("direction");
	if (!directionStr.isEmpty())
	{
		Scanner scanner(directionStr, *this);
		scanner.nextFloat(direction[0]);
		scanner.nextFloat(direction[1]);
		scanner.nextFloat(direction[2]);
	}

	GMString speedStr = e->Attribute("speed");
	if (!speedStr.isEmpty())
		speed = parseFloat(speedStr);

	GMString waveLengthStr = e->Attribute("waveLength");
	if (!waveLengthStr.isEmpty())
		waveLength = parseFloat(waveLengthStr);

	desc.steepness = steepness;
	desc.amplitude = amplitude;
	desc.direction[0] = direction[0];
	desc.direction[1] = direction[1];
	desc.direction[2] = direction[2];
	desc.speed = speed;
	desc.waveLength = waveLength;
}

void Timeline::addObject(AutoReleasePtr<GMGameObject>* object, GMXMLElement*, Action& action)
{
	action.action = [this, object]() {
		GMGameObject* obj = object->release();
		m_world->addObjectAndInit(obj);
		m_world->addToRenderList(obj);
	};
}

void Timeline::addObject(AutoReleasePtr<ILight>* light, GMXMLElement*, Action& action)
{
	action.action = [this, light]() {
		m_context->getEngine()->addLight(light->release());
	};
}

void Timeline::addObject(AutoReleasePtr<IParticleSystem>* particles, GMXMLElement* e, Action& action)
{
	action.action = [this, particles]() {
		m_world->getParticleSystemManager()->addParticleSystem(particles->release());
	};
}

void Timeline::addObject(GMShadowSourceDesc* s, GMXMLElement*, Action& action)
{
	action.action = [this, s]() {
		m_context->getEngine()->setShadowSource(*s);
	};
}

void Timeline::removeObject(ILight* light, GMXMLElement* e, Action& action)
{
	GMString object = e->Attribute("object");
	action.action = [this, light, object]() {
		IGraphicEngine* engine = m_context->getEngine();
		if (!engine->removeLight(light))
			gm_warning(gm_dbg_wrap("Cannot remove light: {0}."), object);
	};
}

void Timeline::removeObject(GMGameObject* obj, GMXMLElement* e, Action& action, bool deleteFromMemory)
{
	GMString objName = e->Attribute("object");
	action.action = [this, obj, objName, deleteFromMemory]() {
		if (!m_world->removeFromRenderList(obj))
		{
			gm_warning(gm_dbg_wrap("Cannot find object '{0}' from render list."), objName);
		}

		if (deleteFromMemory)
		{
			if (m_world->removeObject(obj))
			{
				m_objects.erase(objName);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot find object '{0}' in world."), objName);
			}
		}
	};
}

void Timeline::setColorForModel(GMModel* model, const GMfloat color[4])
{
	GMParts& parts = model->getParts();
	GMVertices tmp;
	for (auto iter = parts.begin(); iter != parts.end(); ++iter)
	{
		GMPart* part = *iter;
		part->swap(tmp);
		for (GMVertex& v : tmp)
		{
			v.color = { color[0], color[1], color[2], color[3] };
		}
		part->swap(tmp);
	}
}

void Timeline::transferColorForModel(GMModel* model, const GMfloat color[4])
{
	GMModelDataProxy* proxy = model->getModelDataProxy();
	if (!proxy)
	{
		setColorForModel(model, color);
	}
	else
	{
		GMVertices& cache = m_verticesCache[model];
		GM_ASSERT(model->getVerticesCount() == cache.size());

		proxy->beginUpdateBuffer(GMModelBufferType::VertexBuffer);
		for (GMsize_t i = 0; i < cache.size(); ++i)
		{
			cache[i].color = { color[0], color[1], color[2], color[3] };
		}

		memcpy_s(proxy->getBuffer(), sizeof(GMVertex) * model->getVerticesCount(), cache.data(), sizeof(GMVertex) * cache.size());
		proxy->endUpdateBuffer();
	}
}

CurveType Timeline::parseCurve(GMXMLElement* e, GMInterpolationFunctors& f)
{
	GMString function = e->Attribute("function");
	if (function == L"cubic-bezier")
	{
		GMString controlStr = e->Attribute("control");
		if (!controlStr.isEmpty())
		{
			GMfloat cpx0, cpy0, cpx1, cpy1;
			Scanner scanner(controlStr, *this);
			scanner.nextFloat(cpx0);
			scanner.nextFloat(cpy0);
			scanner.nextFloat(cpx1);
			scanner.nextFloat(cpy1);
			f = GMInterpolationFunctors::getDefaultInterpolationFunctors();
			f.vec3Functor = GMSharedPtr<IInterpolationVec3>(new GMCubicBezierFunctor<GMVec3>(GMVec2(cpx0, cpy0), GMVec2(cpx1, cpy1)));
			f.vec4Functor = GMSharedPtr<IInterpolationVec4>(new GMCubicBezierFunctor<GMVec4>(GMVec2(cpx0, cpy0), GMVec2(cpx1, cpy1)));
			return CurveType::CubicBezier;
		}
		else
		{
			gm_warning(gm_dbg_wrap("Cubic bezier missing control points. Object for {0}"), GMString(e->Attribute("object")));
		}
	}
	return CurveType::NoCurve;
}

void Timeline::bindAction(const Action& a)
{
	if (a.runType == Action::Immediate)
		m_immediateActions.insert(a);
	else
		m_deferredActions.insert(a);
}

void Timeline::runImmediateActions()
{
	for (auto iter = m_immediateActions.begin(); iter != m_immediateActions.end(); )
	{
		auto action = *iter;
		if (action.runType == Action::Immediate)
		{
			if (action.action)
				action.action();
		}
		iter = m_immediateActions.erase(iter);
	}
}

void Timeline::runActions()
{
	while (m_currentAction != m_deferredActions.end())
	{
		GM_ASSERT(m_currentAction->runType == Action::Deferred);
		auto next = m_currentAction;
		++next;
		if (next == m_deferredActions.end())
		{
			// 当前为最后一帧
			if (m_timeline >= m_currentAction->timePoint)
			{
				if (m_currentAction->action)
					m_currentAction->action();
			}
			else
			{
				break;
			}
		}
		else
		{
			// 当前不是最后一帧
			if (m_currentAction->timePoint <= m_timeline)
			{
				if (m_currentAction->action)
					m_currentAction->action();
			}
			else
			{
				break;
			}
		}

		++m_currentAction;
	}
	
	if (m_currentAction == m_deferredActions.end())
	{
		m_finished = true;
		m_playing = false;
	}
}

AssetType Timeline::getAssetType(const GMString& objectName, OUT void** out)
{
	AssetType result = AssetType::NotFound;
	if (objectName == L"$camera")
	{
		result = AssetType::Camera;
		if (out)
			*out = &m_context->getEngine()->getCamera();
	}
	else
	{
		do 
		{
			if (getAssetAndType<AssetType::GameObject>(m_objects, objectName, result, out))
				break;
			if (getAssetAndType<AssetType::Light>(m_lights, objectName, result, out))
				break;
			if (getAssetAndType<AssetType::AudioSource>(m_audioSources, objectName, result, out))
				break;
			if (getAssetAndType<AssetType::Particles>(m_particleSystems, objectName, result, out))
				break;
			if (getAssetAndType<AssetType::Shadow>(m_shadows, objectName, result, out))
				break;
		} while (false);
	}
	return result;
}

void Timeline::playAudio(IAudioSource* source)
{
	source->play(false);
}

void Timeline::play(GMGameObject* obj, const GMString& name)
{
	if (!name.isEmpty())
	{
		GMsize_t idx = obj->getAnimationIndexByName(name);
		if (idx == -1)
		{
			gm_warning(gm_dbg_wrap("Cannot find animation name of object: {0}"), name);
		}
		else
		{
			obj->setAnimation(idx);
		}
	}
	obj->reset(true);
	obj->play();
}

GMint32 Timeline::parseInt(const GMString& str, bool* ok)
{
	GMString temp = str;
	temp = getValueFromDefines(temp);
	return GMString::parseInt(temp, ok);
}

GMfloat Timeline::parseFloat(const GMString& str, bool* ok)
{
	GMString temp = str;
	temp = getValueFromDefines(temp);
	return GMString::parseFloat(temp, ok);
}

bool Timeline::objectExists(const GMString& id)
{
	bool exists = !(m_objects.find(id) == m_objects.end());
	if (exists)
	{
		if (!m_supressedWarnings.isSet(Warning_ObjectExistsWarning))
			gm_warning(gm_dbg_wrap("Object exists: '{0}'"), id);
	}
	return exists;
}

Scanner::Scanner(const GMString& line, Timeline& timeline)
	: m_timeline(timeline)
	, m_scanner(line)
{
}

bool Scanner::nextInt(REF GMint32& value)
{
	GMString str;
	m_scanner.next(str);
	if (str.isEmpty())
		return false;

	value = m_timeline.parseInt(str);
	return true;
}

bool Scanner::nextFloat(REF GMfloat& value)
{
	GMString str;
	m_scanner.next(str);
	if (str.isEmpty())
		return false;

	value = m_timeline.parseFloat(str);
	return true;
}

Slots::Slots(Slots* parent)
	: m_parent(parent)
{
	if (parent)
		parent->append(this);
}

Slots::~Slots()
{
	while (!m_children.empty())
	{
		m_children.front()->destroy();
	}
}

Slots* Slots::getParent()
{
	return m_parent;
}

bool Slots::addSlot(const GMString& name, const GMString& value)
{
	if (name.isEmpty())
	{
		gm_warning(gm_dbg_wrap("slot name is empty."));
		return false;
	}

	auto result = m_slots.insert(std::make_pair(name, value));
	if (!result.second)
		gm_warning(gm_dbg_wrap("Slot name exists: '{0}', add failed."), name);
	return result.second;
}

bool Slots::getSlotByName(const GMString& name, GMString& result)
{
	auto iter = m_slots.find(name);
	bool notFound = (iter == m_slots.end());
	if (!notFound)
	{
		result = iter->second;
		return true;
	}
	else
	{
		if (getParent())
			return getParent()->getSlotByName(name, result);
		return false;
	}
}

void Slots::destroy()
{
	if (getParent())
		getParent()->remove(this);
	delete this;
}

void Slots::append(Slots* slots)
{
	m_children.push_back(slots);
}

void Slots::remove(Slots* slots)
{
	m_children.remove(slots);
}

SlotsStack::SlotsStack()
	: m_currentSlots(nullptr)
	, m_root(new Slots(nullptr))
{
	m_currentSlots = m_root.get();
}

void SlotsStack::pushSlots()
{
	m_currentSlots = new Slots(m_currentSlots);
}

void SlotsStack::popSlots()
{
	if (m_currentSlots->getParent())
	{
		Slots* tmp = m_currentSlots->getParent();
		m_currentSlots->destroy();
		m_currentSlots = tmp;
	}
	else
	{
		GM_ASSERT(m_currentSlots == m_root.get());
	}
}

bool SlotsStack::addSlot(const GMString& name, const GMString& value)
{
	return m_currentSlots->addSlot(name, value);
}

bool SlotsStack::getSlotByName(const GMString& name, GMString& result)
{
	return m_currentSlots->getSlotByName(name, result);
}

SupressedWarnings::SupressedWarnings()
{
	GM_ZeroMemory(&m_data[0], sizeof(m_data));
}

void SupressedWarnings::set(Warning w)
{
	++m_data[w];
}

bool SupressedWarnings::isSet(Warning w)
{
	return m_data[w] > 0;
}

void SupressedWarnings::clear(Warning w)
{
	if (m_data[w] > 0)
		--m_data[w];
}
