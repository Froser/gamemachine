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

namespace
{
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

void Timeline::parseElements(GMXMLElement* e)
{
	while (e)
	{
		GMString name = e->Name();
		if (name == L"assets")
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
			if (typeStr == "point")
				type = GMLightType::PointLight;
			else if (typeStr == "directional")
				type = GMLightType::DirectionalLight;
			else if (typeStr == "spotlight")
				type = GMLightType::Spotlight;
			else
				gm_warning(gm_dbg_wrap("Wrong light type: {0}"), typeStr);

			ILight* light = nullptr;
			GM.getFactory()->createLight(type, &light);

			GM_ASSERT(light);
			GMString posStr = e->Attribute("position");
			if (!posStr.isEmpty())
			{
				GMScanner scanner(posStr);
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
				GMScanner scanner(ambientStr);
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
				GMScanner scanner(diffuseStr);
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
				GMScanner scanner(specularStr);
				scanner.nextFloat(specular);
				bool bSuc = light->setLightAttribute(GMLight::SpecularIntensity, specular);
				GM_ASSERT(bSuc);
			}

			GMString cutOffStr = e->Attribute("cutoff");
			if (!cutOffStr.isEmpty())
			{
				GMfloat cutOff = 0;
				GMScanner scanner(cutOffStr);
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
		}
		else if (name == L"quad")
		{
			GMString lengthStr = e->Attribute("length");
			GMString widthStr = e->Attribute("width");
			GMfloat length = 0, width = 0;
			length = GMString::parseFloat(lengthStr);
			width = GMString::parseFloat(widthStr);
			
			GMSceneAsset scene;
			GMPrimitiveCreator::createQuadrangle(GMVec2(length / 2, width / 2), 0, scene);

			GMGameObject* obj = new GMGameObject(scene);
			GM_ASSERT(obj->getModel());
			obj->getModel()->getShader().setCull(GMS_Cull::None);
			m_objects[id] = AutoReleasePtr<GMGameObject>(obj);

			parseTextures(obj, e);
			parseMaterial(obj, e);
			parseTransform(obj, e);
		}
		else if (name == L"wave")
		{
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
		}
		else if (name == L"terrain")
		{
			GMfloat terrainX = 0, terrainZ = 0;
			GMfloat width = 1, height = 1, heightScaling = 10.f;

			GMint32 sliceX = 10, sliceY = 10;
			GMfloat texLen = 10, texHeight = 10;

			terrainX = GMString::parseFloat(e->Attribute("terrainX"));
			terrainZ = GMString::parseFloat(e->Attribute("terrainZ"));
			width = GMString::parseFloat(e->Attribute("width"));
			height = GMString::parseFloat(e->Attribute("height"));
			heightScaling = GMString::parseFloat(e->Attribute("heightScaling"));

			GMString sliceStr = e->Attribute("slice");
			if (!sliceStr.isEmpty())
			{
				GMScanner scanner(sliceStr);
				scanner.nextInt(sliceX);
				scanner.nextInt(sliceY);
			}

			GMString texSizeStr = e->Attribute("textureSize");
			if (!texSizeStr.isEmpty())
			{
				GMScanner scanner(texSizeStr);
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
		}
		else if (name == L"shadow")
		{
			GMRect rc = m_context->getWindow()->getRenderRect();
			GMfloat width = rc.width, height = rc.height;
			{
				GMString str = e->Attribute("width");
				if (!str.isEmpty())
					width = GMString::parseFloat(str);
			}
			{
				GMString str = e->Attribute("height");
				if (!str.isEmpty())
					height = GMString::parseFloat(str);
			}

			GMfloat cascades = 1;
			{
				GMString str = e->Attribute("cascades");
				if (!str.isEmpty())
					cascades = GMString::parseFloat(str);
			}

			GMfloat bias = .005f;
			{
				GMString str = e->Attribute("bias");
				if (!str.isEmpty())
					bias = GMString::parseFloat(str);
			}

			Vector<GMfloat> partitions;
			if (cascades > 1)
			{
				partitions.resize(cascades);
				GMString str = e->Attribute("partitions");
				if (!str.isEmpty())
				{
					GMScanner scanner(str);
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
		GMScanner scanner(str);
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
		GMScanner scanner(str);
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
		GMScanner scanner(str);
		GMfloat x, y, z;
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		focus = Normalize(GMVec3(x, y, z));
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
		GMScanner scanner(ambientStr);
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
		GMScanner scanner(diffuseStr);
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
		GMScanner scanner(specularStr);
		scanner.nextFloat(specular);
		component |= GMLightKeyframeComponent::Specular;
	}

	GMString cutOffStr = e->Attribute("cutoff");
	if (!cutOffStr.isEmpty())
	{
		GMScanner scanner(specularStr);
		scanner.nextFloat(cutOff);
		component |= GMLightKeyframeComponent::CutOff;
	}

	GMString posStr = e->Attribute("position");
	if (!posStr.isEmpty())
	{
		GMScanner scanner(posStr);
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
		GMScanner scanner(str);
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
		GMScanner scanner(str);
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
		GMScanner scanner(str);
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
						action.timePoint = m_lastTime + GMString::parseFloat(time);
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
					action.timePoint = GMString::parseFloat(time);
					if (action.timePoint > m_lastTime)
						m_lastTime = action.timePoint;
				}
			}

			if (action.timePoint == 0)
				action.runType = Action::Immediate;

			GMString type = e->Attribute("type");
			if (type == "camera")
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
					endTime = GMString::parseFloat(endTimeStr, &ok);
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
				if (assetType == AssetType::Light)
					removeObject(asset_cast<ILight>(targetObject)->get(), e, action);
				else if (assetType == AssetType::GameObject)
					removeObject(asset_cast<GMGameObject>(targetObject)->get(), e, action);
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
					playAudio(asset_cast<IAudioSource>(targetObject)->get());
				}
				else if (assetType == AssetType::GameObject)
				{
					play(asset_cast<GMGameObject>(targetObject)->get());
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
			else
			{
				gm_warning(gm_dbg_wrap("action type cannot be recognized: {0}"), type);
			}
		}
		else if (name == L"checkpoint")
		{
			GMString type = e->Attribute("type");
			if (type == "save")
			{
				m_checkpointTime = m_lastTime;
			}
			else if (type == "load")
			{
				m_lastTime = m_checkpointTime;
			}
			else if (type == "time")
			{
				GMString timeStr = e->Attribute("time");
				if (!timeStr.isEmpty())
				{
					GMfloat time = 0;
					bool ok = false;
					time = GMString::parseFloat(timeStr, &ok);
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
			fovy = Radian(GMString::parseFloat(fovyStr));

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
			n = GMString::parseFloat(nStr);

		fStr = e->Attribute("far");
		if (fStr.isEmpty())
			f = 3200;
		else
			f = GMString::parseFloat(fStr);

		cp.fovy = fovy;
		cp.aspect = aspect;
		cp.n = n;
		cp.f = f;
		component |= PerspectiveComponent;
	}
	else if (view == "ortho")
	{
		GMfloat left = 0, right = 0, top = 0, bottom = 0;
		GMfloat n = 0, f = 0;
		left = GMString::parseInt(e->Attribute("left"));
		right = GMString::parseInt(e->Attribute("right"));
		top = GMString::parseInt(e->Attribute("top"));
		bottom = GMString::parseInt(e->Attribute("bottom"));
		n = GMString::parseInt(e->Attribute("near"));
		f = GMString::parseInt(e->Attribute("far"));
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
		GMScanner scanner(posStr);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		position = GMVec3(x, y, z);
	}

	if (!dirStr.isEmpty())
	{
		GMfloat x = 0, y = 0, z = 0;
		GMScanner scanner(dirStr);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		direction = GMVec3(x, y, z);
	}

	if (!focusStr.isEmpty())
	{
		GMfloat x = 0, y = 0, z = 0;
		GMScanner scanner(focusStr);
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
		GMScanner scanner(translateStr);
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
		GMScanner scanner(str);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		o->setScaling(Scale(GMVec3(x, y, z)));
	}

	str = e->Attribute("translate");
	if (!str.isEmpty())
	{
		GMfloat x, y, z;
		GMScanner scanner(str);
		scanner.nextFloat(x);
		scanner.nextFloat(y);
		scanner.nextFloat(z);
		o->setTranslation(Translate(GMVec3(x, y, z)));
	}

	str = e->Attribute("rotate");
	if (!str.isEmpty())
	{
		GMfloat x, y, z, degree;
		GMScanner scanner(str);
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

	GMString id = e->Attribute("id");
	GMModel* model = o->getModel();
	if (!model)
		return;

	GMShader& shader = model->getShader();
	parseTexture(shader, e, "ambient", GMTextureType::Ambient);
	parseTexture(shader, e, "diffuse", GMTextureType::Diffuse);
	parseTexture(shader, e, "normal", GMTextureType::NormalMap);
	parseTexture(shader, e, "specular", GMTextureType::Specular);

	{
		GMString pbrStr = e->Attribute("pbr");
		if (!pbrStr.isEmpty())
		{
			const PBR* pbr = findPBR(pbrStr);
			if (pbr)
			{
				shader.setIlluminationModel(gm::GMIlluminationModel::CookTorranceBRDF);
				gm::GMToolUtil::addTextureToShader(shader, pbr->albedo, gm::GMTextureType::Albedo);
				gm::GMToolUtil::addTextureToShader(shader, pbr->normal, gm::GMTextureType::NormalMap);
				gm::GMToolUtil::addTextureToShader(shader, pbr->metallicRoughnessAO, gm::GMTextureType::MetallicRoughnessAO);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot find pbr assets: {0}"), pbrStr);
			}
		}
	}

	parseTextureTransform(shader, e, "ambientTranslateSpeed", GMTextureType::Ambient, GMS_TextureTransformType::Scroll);
	parseTextureTransform(shader, e, "diffuseTranslateSpeed", GMTextureType::Diffuse, GMS_TextureTransformType::Scroll);
	parseTextureTransform(shader, e, "normalTranslateSpeed", GMTextureType::NormalMap, GMS_TextureTransformType::Scroll);
	parseTextureTransform(shader, e, "specularTranslateSpeed", GMTextureType::Specular, GMS_TextureTransformType::Scroll);
}

void Timeline::parseMaterial(GMGameObject* o, GMXMLElement* e)
{
	if (!o)
		return;

	GMModel* model = o->getModel();
	if (!model)
		return;

	GMShader& shader = model->getShader();

	GMfloat x = 0, y = 0, z = 0;

	{
		GMString str = e->Attribute("ka");
		if (!str.isEmpty())
		{
			GMScanner scanner(str);
			scanner.nextFloat(x);
			scanner.nextFloat(y);
			scanner.nextFloat(z);
			shader.getMaterial().setAmbient(GMVec3(x, y, z));
		}
	}

	{
		GMString str = e->Attribute("kd");
		if (!str.isEmpty())
		{
			GMScanner scanner(str);
			scanner.nextFloat(x);
			scanner.nextFloat(y);
			scanner.nextFloat(z);
			shader.getMaterial().setDiffuse(GMVec3(x, y, z));
		}
	}

	{
		GMString str = e->Attribute("ks");
		if (!str.isEmpty())
		{
			GMScanner scanner(str);
			scanner.nextFloat(x);
			scanner.nextFloat(y);
			scanner.nextFloat(z);
			shader.getMaterial().setSpecular(GMVec3(x, y, z));
		}
	}

	{
		GMString str = e->Attribute("shininess");
		if (!str.isEmpty())
		{
			shader.getMaterial().setShininess(GMString::parseFloat(str));
		}
	}
}

void Timeline::parseTexture(GMShader& shader, GMXMLElement* e, const char* type, GMTextureType textureType)
{
	GMString tex = e->Attribute(type);
	if (!tex.isEmpty())
	{
		GMAsset asset = findAsset(tex);
		if (!asset.isEmpty() && asset.isTexture())
			GMToolUtil::addTextureToShader(shader, asset, textureType);
		else
			gm_warning(gm_dbg_wrap("Cannot find texture asset: {0}"), tex);
	}
}

void Timeline::parseTextureTransform(GMShader& shader, GMXMLElement* e, const char* type, GMTextureType textureType, GMS_TextureTransformType transformType)
{
	GMString val = e->Attribute(type);
	if (!val.isEmpty())
	{
		GMScanner scanner(val);
		GMS_TextureTransform tt;
		tt.type = transformType;
		scanner.nextFloat(tt.p1);
		scanner.nextFloat(tt.p2);
		shader.getTextureList().getTextureSampler(textureType).setTextureTransform(0, tt);
	}
}

void Timeline::parseAttributes(GMGameObject* obj, GMXMLElement* e, Action& action)
{
	GM_ASSERT(obj);
	{
		GMString value = e->Attribute("visible");
		bool visible = toBool(value);
		action.action = [obj, visible]() {
			obj->setVisible(visible);
		};
	}
}

void Timeline::parseWaveObjectAttributes(GMWaveGameObjectDescription& desc, GMXMLElement* e)
{
	GMfloat terrainX = 0, terrainZ = 0;
	GMfloat terrainLength = 1, terrainWidth = 1, heightScaling = 10.f;

	GMint32 sliceX = 10, sliceY = 10;
	GMfloat texLen = 10, texHeight = 10;

	GMfloat texScaleLen = 2, texScaleHeight = 2;

	terrainX = GMString::parseFloat(e->Attribute("terrainX"));
	terrainZ = GMString::parseFloat(e->Attribute("terrainZ"));
	terrainLength = GMString::parseFloat(e->Attribute("length"));
	terrainWidth = GMString::parseFloat(e->Attribute("width"));
	heightScaling = GMString::parseFloat(e->Attribute("heightScaling"));

	GMString sliceStr = e->Attribute("slice");
	if (!sliceStr.isEmpty())
	{
		GMScanner scanner(sliceStr);
		scanner.nextInt(sliceX);
		scanner.nextInt(sliceY);
	}

	GMString texSizeStr = e->Attribute("textureSize");
	if (!texSizeStr.isEmpty())
	{
		GMScanner scanner(texSizeStr);
		scanner.nextFloat(texLen);
		scanner.nextFloat(texHeight);
	}

	GMString texScaling = e->Attribute("textureScaling");
	if (!texScaling.isEmpty())
	{
		GMScanner scanner(texScaling);
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
		steepness = GMString::parseFloat(steepnessStr);

	GMString amplitudeStr = e->Attribute("amplitude");
	if (!amplitudeStr.isEmpty())
		amplitude = GMString::parseFloat(amplitudeStr);

	GMString directionStr = e->Attribute("direction");
	if (!directionStr.isEmpty())
	{
		GMScanner scanner(directionStr);
		scanner.nextFloat(direction[0]);
		scanner.nextFloat(direction[1]);
		scanner.nextFloat(direction[2]);
	}

	GMString speedStr = e->Attribute("speed");
	if (!speedStr.isEmpty())
		speed = GMString::parseFloat(speedStr);

	GMString waveLengthStr = e->Attribute("waveLength");
	if (!waveLengthStr.isEmpty())
		waveLength = GMString::parseFloat(waveLengthStr);

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

void Timeline::removeObject(GMGameObject* obj, GMXMLElement* e, Action& action)
{
	GMString objName = e->Attribute("object");
	action.action = [this, obj, objName]() {
		if (!m_world->removeFromRenderList(obj))
		{
			gm_warning(gm_dbg_wrap("Cannot find object '{0}' from render list."), objName);
		}
	};
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
			GMScanner scanner(controlStr);
			scanner.nextFloat(cpx0);
			scanner.nextFloat(cpy0);
			scanner.nextFloat(cpx1);
			scanner.nextFloat(cpy1);
			f = GMInterpolationFunctors::getDefaultInterpolationFunctors();
			f.vec3Functor = GMSharedPtr<IInterpolationVec3>(new GMCubicBezierFunctor<GMVec3>(GMVec2(cpx0, cpy0), GMVec2(cpx1, cpy1)));
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
	for (const auto& action : m_immediateActions)
	{
		if (action.runType == Action::Immediate)
		{
			if (action.action)
				action.action();
		}
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
	if (objectName == "$camera")
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

void Timeline::play(GMGameObject* obj)
{
	if (GMWaveGameObject* wave = dynamic_cast<GMWaveGameObject*>(obj))
	{
		wave->play();
	}
}
