#include "stdafx.h"
#include "timeline.h"
#include <gmimagebuffer.h>
#include <gmimage.h>
#include <gmgameobject.h>
#include <gmlight.h>
#include <gmmodelreader.h>
#include <gmutilities.h>
#include <gmgraphicengine.h>

#define NoCameraComponent 0
#define PositionComponent 0x01
#define DirectionComponent 0x02
#define FocusAtComponent 0x04

#define PerspectiveComponent 0x01
#define CameraLookAtComponent 0x02

struct CameraParams
{
	GMfloat fovy, aspect, n, f;
};

namespace
{
	template <AssetType Type, typename Container>
	bool getAssetAndType(const Container& container, const GMString& objectName, REF AssetType& result, OUT void** out)
	{
		result = AssetType::NotFound;

		decltype(container.end()) iter;
		if ((iter = container.find(objectName)) != container.end())
		{
			result = Type;
			if (out)
				(*out) = iter->second;
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

Timeline::Timeline(const IRenderContext* context, GMGameWorld* world)
	: m_context(context)
	, m_world(world)
	, m_timeline(0)
	, m_playing(false)
	, m_finished(false)
{
	GM_ASSERT(m_context && m_world);
	m_animations.resize(EndOfAnimation);
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

	for (auto& animation : m_animations)
	{
		animation.update(dt);
	}
}

void Timeline::play()
{
	m_currentAction = m_deferredActions.begin();
	m_playing = true;
	for (auto& animation : m_animations)
	{
		animation.play();
	}
}

void Timeline::pause()
{
	m_playing = false;
	for (auto& animation : m_animations)
	{
		animation.pause();
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

			m_lights[id] = light;
		}
		else if (name == L"cubemap")
		{
			GMGameObject* obj = nullptr;
			GMString assetName = e->Attribute("asset");
			GMAsset asset = findAsset(assetName);
			if (!asset.isEmpty())
				obj = m_objects[id] = new GMCubeMapGameObject(asset);
			else
				gm_warning(gm_dbg_wrap("Cannot find asset: {0}"), assetName);

			parseTransform(obj, e);
		}
		else if (name == L"object")
		{
			GMGameObject* obj = nullptr;
			GMString assetName = e->Attribute("asset");
			GMAsset asset = findAsset(assetName);
			if (!asset.isEmpty())
				obj = m_objects[id] = new GMGameObject(asset);
			else
				gm_warning(gm_dbg_wrap("Cannot find asset: {0}"), assetName);

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
				GMGameObject* obj = m_objects[id] = new GMGameObject(scene);
				parseTextures(obj, e);
				parseMaterial(obj, e);
				parseTransform(obj, e);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Create terrain error. Id: {0}"), id);
			}
		}
		e = e->NextSiblingElement();
	}
}

void Timeline::interpolateCamera(GMXMLElement* e, Action& action)
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
		gm_warning(gm_dbg_wrap("You cannot specify both 'direction' and 'focus'. 'direction' won't work."));
	}

	if (component != NoCameraComponent)
	{
		GMfloat t = action.timePoint;
		action.runType = Action::Immediate; // 插值动作的添加是立即的
		
		GMInterpolationFunctors f;
		CurveType curve = parseCurve(e, f);
		action.action = [this, component, pos, dir, focus, t, curve, f]() {
			GMAnimation& animation = m_animations[Camera];
			GMCamera& camera = m_context->getEngine()->getCamera();
			const GMCameraLookAt& lookAt = camera.getLookAt();
			animation.setTargetObjects(&camera);
			
			GMVec3 posCandidate = (component & PositionComponent) ? pos : lookAt.position;
			GMVec3 dirCandidate = (component & DirectionComponent) ? dir : lookAt.lookDirection;
			GMVec3 focusCandidate = (component & FocusAtComponent) ? focus : lookAt.position + lookAt.lookDirection;
			GMCameraKeyframe* keyframe = nullptr;
			if (component & DirectionComponent)
				keyframe = new GMCameraKeyframe(GMCameraKeyframeComponent::LookAtDirection, posCandidate, dirCandidate, t);
			else // 默认是按照focusAt调整视觉
				keyframe = new GMCameraKeyframe(GMCameraKeyframeComponent::FocusAt, posCandidate, focusCandidate, t);
			animation.addKeyFrame(keyframe);
			if (curve != CurveType::NoCurve)
				keyframe->setFunctors(f);
		};
		bindAction(action);
	}
}

void Timeline::interpolateLight(GMXMLElement* e, Action& action, ILight* light)
{
	GMint32 component = GMLightKeyframeComponent::NoComponent;
	GMVec3 ambient, diffuse;
	GMfloat specular = 0;

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

	if (component != GMLightKeyframeComponent::NoComponent)
	{
		GMfloat t = action.timePoint;
		action.runType = Action::Immediate; // 插值动作的添加是立即的
		GMInterpolationFunctors f;
		CurveType curve = parseCurve(e, f);
		action.action = [this, light, component, ambient, diffuse, specular, t, curve, f]() {
			GMAnimation& animation = m_animations[Light];
			animation.setTargetObjects(light);
			GMAnimationKeyframe* keyframe = new GMLightKeyframe(m_context, component, ambient, diffuse, specular, t);
			if (curve != CurveType::NoCurve)
				keyframe->setFunctors(f);
			animation.addKeyFrame(keyframe);
		};
		bindAction(action);
	}
}

void Timeline::parseActions(GMXMLElement* e)
{
	GM_ASSERT(m_world);
	while (e)
	{
		GMString name = e->Name();
		if (name == L"action")
		{
			Action action = { Action::Immediate };
			GMString time = e->Attribute("time");
			if (!time.isEmpty())
				action.runType = Action::Deferred;

			GMString type = e->Attribute("type");
			if (type == "camera")
			{
				CameraParams p;
				GMCameraLookAt lookAt;
				GMint32 component = parseCameraAction(e, p, lookAt);
				
				if (component & PerspectiveComponent)
				{
					action.action = [p, this]() {
						auto& camera = m_context->getEngine()->getCamera();
						camera.setPerspective(p.fovy, p.aspect, p.n, p.f);
					};
				}
				if (component & CameraLookAtComponent)
				{
					action.action = [p, action, lookAt, this]() {
						if (action.action)
							action.action();

						auto& camera = m_context->getEngine()->getCamera();
						camera.lookAt(lookAt);
					};
				}

				bindAction(action);
			}
			else if (type == L"addObject")
			{
				GMString object = e->Attribute("object");
				auto objectIter = m_objects.find(object);
				if (objectIter != m_objects.end())
				{
					GMGameObject* targetObject = objectIter->second;
					action.action = [this, targetObject]() {
						m_world->addObjectAndInit(targetObject);
						m_world->addToRenderList(targetObject);
					};

					bindAction(action);
				}
				else
				{
					gm_warning(gm_dbg_wrap("Cannot find object: {0}"), object);
				}
			}
			else if (type == L"addLight")
			{
				GMString object = e->Attribute("object");
				auto lightIter = m_lights.find(object);
				if (lightIter != m_lights.end())
				{
					ILight* targetLight = lightIter->second;
					action.action = [this, targetLight]() {
						m_context->getEngine()->addLight(targetLight);
					};
					
					bindAction(action);
				}
				else
				{
					gm_warning(gm_dbg_wrap("Cannot find light: {0}"), object);
				}
			}
			else if (type == L"animate") // 或者是其它插值变换
			{
				if (!time.isEmpty())
				{
					GMfloat t = GMString::parseFloat(time);
					action.timePoint = t;

					GMString object = e->Attribute("object");

					void* targetObject = nullptr;
					AssetType assetType = getAssetType(object, &targetObject);

					if (assetType == AssetType::Camera)
					{
						interpolateCamera(e, action);
					}
					else if (assetType == AssetType::Light)
					{
						interpolateLight(e, action, static_cast<ILight*>(targetObject));
					}
					else
					{
						gm_warning(gm_dbg_wrap("Cannot find object: {0}"), object);
					}
				}
				else
				{
					gm_warning(gm_dbg_wrap("type 'animate' must combine with attribute 'time'."));
				}
			}
			else if (type == L"shadow")
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

				action.action = [this, component, cascades, width, height, bias, partitions = std::move(partitions), cp, lookAt]() {
					GMShadowSourceDesc desc;
					desc.type = GMShadowSourceDesc::CSMShadow;
					GMCamera camera;
					if (component & PerspectiveComponent)
						camera.setPerspective(cp.fovy, cp.aspect, cp.n, cp.f);
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
					m_context->getEngine()->setShadowSource(desc);
				};
				bindAction(action);
			}
			else if (type == L"attribute")
			{
				GMString object = e->Attribute("object");
				void* targetObject = nullptr;
				AssetType assetType = getAssetType(object, &targetObject);
				if (assetType == AssetType::GameObject)
					parseAttributes(static_cast<GMGameObject*>(targetObject), e, action);
				else if (assetType == AssetType::NotFound)
					gm_warning(gm_dbg_wrap("Object '{0}' not found."), object);
				else
					gm_warning(gm_dbg_wrap("Object '{0}' doesn't support 'attribute' type."), object);
				action.timePoint = GMString::parseFloat(time);
				bindAction(action);
			}
			else if (type == L"removeObject")
			{
				GMString object = e->Attribute("object");
				void* targetObject = nullptr;
				AssetType assetType = getAssetType(object, &targetObject);
				if (assetType == AssetType::Light)
					removeObject(static_cast<ILight*>(targetObject), e, action);
				else if (assetType == AssetType::NotFound)
					gm_warning(gm_dbg_wrap("Object '{0}' not found."), object);
				else
					gm_warning(gm_dbg_wrap("Object '{0}' doesn't support 'removeObject' type."), object);
				action.timePoint = GMString::parseFloat(time);
				bindAction(action);
			}
			else
			{
				gm_warning(gm_dbg_wrap("action type cannot be recognized: {0}"), type);
			}
		}
		else
		{
			gm_warning(gm_dbg_wrap("tag name cannot be recognized: {0}"), e->Name());
		}

		e = e->NextSiblingElement();
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

		cp = { fovy, aspect, n, f };
		component |= PerspectiveComponent;
	}
	else if (!view.isEmpty())
	{
		gm_warning(gm_dbg_wrap("Camera view only supports 'perspective'"));
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

GMAsset Timeline::findAsset(const GMString& assetName)
{
	auto assetIter = m_assets.find(assetName);
	if (assetIter != m_assets.end())
		return assetIter->second;

	return GMAsset::invalidAsset();
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

	{
		GMString tex = e->Attribute("ambient");
		if (!tex.isEmpty())
		{
			GMAsset asset = findAsset(tex);
			if (!asset.isEmpty() && asset.isTexture())
				GMToolUtil::addTextureToShader(shader, asset, GMTextureType::Ambient);
			else
				gm_warning(gm_dbg_wrap("Cannot find texture asset: {0}"), tex);
		}
	}

	{
		GMString tex = e->Attribute("diffuse");
		if (!tex.isEmpty())
		{
			GMAsset asset = findAsset(tex);
			if (!asset.isEmpty() && asset.isTexture())
				GMToolUtil::addTextureToShader(shader, asset, GMTextureType::Diffuse);
			else
				gm_warning(gm_dbg_wrap("Cannot find texture asset: {0}"), tex);
		}
	}

	{
		GMString tex = e->Attribute("specular");
		if (!tex.isEmpty())
		{
			GMAsset asset = findAsset(tex);
			if (!asset.isEmpty() && asset.isTexture())
				GMToolUtil::addTextureToShader(shader, asset, GMTextureType::Specular);
			else
				gm_warning(gm_dbg_wrap("Cannot find texture asset: {0}"), tex);
		}
	}

	{
		GMString tex = e->Attribute("normal");
		if (!tex.isEmpty())
		{
			GMAsset asset = findAsset(tex);
			if (!asset.isEmpty() && asset.isTexture())
				GMToolUtil::addTextureToShader(shader, asset, GMTextureType::NormalMap);
			else
				gm_warning(gm_dbg_wrap("Cannot find texture asset: {0}"), tex);
		}
	}

	{
		GMString tex = e->Attribute("albedo");
		if (!tex.isEmpty())
		{
			GMAsset asset = findAsset(tex);
			if (!asset.isEmpty() && asset.isTexture())
				GMToolUtil::addTextureToShader(shader, asset, GMTextureType::Albedo);
			else
				gm_warning(gm_dbg_wrap("Cannot find texture asset: {0}"), tex);
		}
	}

	//TODO AO, etc
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

void Timeline::removeObject(ILight* light, GMXMLElement* e, Action& action)
{
	GMString object = e->Attribute("object");
	action.action = [this, light, object]() {
		IGraphicEngine* engine = m_context->getEngine();
		if (!engine->removeLight(light))
			gm_warning(gm_dbg_wrap("Cannot remove light: {0}."), object);
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
				m_currentAction->action();
			else
				break;
		}
		else
		{
			// 当前不是最后一帧
			if (m_currentAction->timePoint <= m_timeline && m_timeline <= next->timePoint)
				m_currentAction->action();
			else if (m_currentAction->timePoint > m_timeline)
				break;
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
		} while (false);
	}
	return result;
}
