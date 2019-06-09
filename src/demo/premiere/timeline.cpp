#include "stdafx.h"
#include "timeline.h"
#include <gmimagebuffer.h>
#include <gmimage.h>
#include <gmgameobject.h>
#include <gmlight.h>
#include <gmmodelreader.h>
#include <gmutilities.h>
#include <gmgraphicengine.h>

#define NoComponent 0
#define PositionComponent 0x01
#define DirectionComponent 0x02
#define FocusAtComponent 0x04

enum Animation
{
	Camera,
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
	m_currentAction = m_actions.begin();
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
				GMfloat lightPos[] = { x, y, z };
				light->setLightAttribute3(GMLight::Position, lightPos);
			}

			GMString ambientStr = e->Attribute("ambient");
			if (!ambientStr.isEmpty())
			{
				GMScanner scanner(ambientStr);
				GMfloat x, y, z;
				scanner.nextFloat(x);
				scanner.nextFloat(y);
				scanner.nextFloat(z);
				GMfloat lightPos[] = { x, y, z };
				light->setLightAttribute3(GMLight::AmbientIntensity, lightPos);
			}

			GMString diffuseStr = e->Attribute("diffuse");
			if (!diffuseStr.isEmpty())
			{
				GMScanner scanner(diffuseStr);
				GMfloat x, y, z;
				scanner.nextFloat(x);
				scanner.nextFloat(y);
				scanner.nextFloat(z);
				GMfloat lightPos[] = { x, y, z };
				light->setLightAttribute3(GMLight::DiffuseIntensity, lightPos);
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
		else if (name == L"quad")
		{
			GMfloat x = 0, y = 0, z = 0;
			GMfloat width = 1, height = 1;
			x = GMString::parseFloat(e->Attribute("x"));
			y = GMString::parseFloat(e->Attribute("y"));
			z = GMString::parseFloat(e->Attribute("z"));
			width = GMString::parseFloat(e->Attribute("width"));
			height = GMString::parseFloat(e->Attribute("height"));

			GMVec2 he(width / 2.f, height / 2.f);
			GMSceneAsset scene;
			GMPrimitiveCreator::createQuadrangle(he, z, scene);

			if (!scene.isEmpty())
			{
				GMGameObject* obj = m_objects[id] = new GMGameObject(scene);
				parseTextures(obj, e);
				parseMaterial(obj, e);
				parseTransform(obj, e);
			}
			else
			{
				gm_warning(gm_dbg_wrap("Create quadrangle error. Id: {0}"), id);
			}
		}
		e = e->NextSiblingElement();
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

					action.action = [fovy, aspect, n, f, this]() {
						auto& camera = m_context->getEngine()->getCamera();
						camera.setPerspective(fovy, aspect, n, f);
					};

					bindAction(action);
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
					action.action = [this, focus, position]() {
						auto& camera = m_context->getEngine()->getCamera();
						GMCameraLookAt lookAt = GMCameraLookAt::makeLookAt(position, focus);
						camera.lookAt(lookAt);
					};
				}
				else if (!dirStr.isEmpty())
				{
					// direction
					action.action = [this, direction, position]() {
						auto& camera = m_context->getEngine()->getCamera();
						GMCameraLookAt lookAt(direction, position);
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
			else if (type == L"lerp")
			{
				if (!time.isEmpty())
				{
					GMint32 component = NoComponent;

					GMfloat t = GMString::parseFloat(time);
					action.timePoint = t;

					GMString object = e->Attribute("object");
					if (object == L"$camera")
					{
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

						if (component != NoComponent)
						{
							action.runType = Action::Immediate; // lerp动作的添加是立即的
							action.action = [this, component, pos, dir, focus, t]() {
								GMAnimation& animation = m_animations[Camera];
								GMCamera& camera = m_context->getEngine()->getCamera();
								const GMCameraLookAt& lookAt = camera.getLookAt();
								animation.setTargetObjects(&camera);

								GMVec3 posCandidate = (component & PositionComponent) ? pos : lookAt.position;
								GMVec3 dirCandidate = (component & DirectionComponent) ? dir : lookAt.lookDirection;
								GMVec3 focusCandidate = (component & FocusAtComponent) ? focus : lookAt.position + lookAt.lookDirection;
								if (component & DirectionComponent)
									animation.addKeyFrame(new GMCameraKeyframe(GMCameraKeyframe::ByPositionAndDirection, posCandidate, dirCandidate, t));
								else // 默认是按照focusAt调整视觉
									animation.addKeyFrame(new GMCameraKeyframe(GMCameraKeyframe::ByPositionAndFocusAt, posCandidate, focusCandidate, t));
							};
							bindAction(action);
						}
					}
				}
				else
				{
					gm_warning(gm_dbg_wrap("type 'lerp' must combine with attribute 'time'."));
				}
			}
			else if (type == L"shadow")
			{
				GMString camera = e->Attribute("camera");
				if (camera != L"$camera" && !camera.isEmpty())
					gm_warning(gm_dbg_wrap("You must set 'camera' attribute as '$camera'."));

				GMint32 component = NoComponent;
				GMfloat posX = 0, posY = 0, posZ = 0;
				GMString posStr = e->Attribute("position");
				{
					GMScanner scanner(posStr);
					scanner.nextFloat(posX);
					scanner.nextFloat(posY);
					scanner.nextFloat(posZ);
				}

				GMString directionStr = e->Attribute("direction");
				GMString focusStr = e->Attribute("focus");
				GMfloat focusX = 0, focusY = 0, focusZ = 0;
				GMfloat dirX = 0, dirY = 0, dirZ = 0;
				if (!focusStr.isEmpty())
				{
					GMScanner scanner(focusStr);
					scanner.nextFloat(focusX);
					scanner.nextFloat(focusY);
					scanner.nextFloat(focusZ);
					component |= FocusAtComponent;
				}

				if (!directionStr.isEmpty())
				{
					GMScanner scanner(directionStr);
					scanner.nextFloat(dirX);
					scanner.nextFloat(dirY);
					scanner.nextFloat(dirZ);
					component |= DirectionComponent;
				}

				if ((component & DirectionComponent) && (component & FocusAtComponent))
				{
					gm_warning(gm_dbg_wrap("You cannot specify both 'direction' and 'focus'. 'direction' won't work."));
				}

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

				action.action = [this, component, posX, posY, posZ, focusX, focusY, focusZ, dirX, dirY, dirZ, cascades, width, height, partitions = std::move(partitions)]() {
					GMShadowSourceDesc desc;
					desc.type = GMShadowSourceDesc::CSMShadow;
					desc.camera = m_context->getEngine()->getCamera();
					desc.position = GMVec4(posX, posY, posZ, 1);
					if (component & FocusAtComponent)
					{
						desc.camera.lookAt(GMCameraLookAt::makeLookAt(desc.position, GMVec3(focusX, focusY, focusZ)));
					}
					else if (component & DirectionComponent)
					{
						desc.camera.lookAt(GMCameraLookAt(desc.position, GMVec3(dirX, dirY, dirZ)));
					}

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
					desc.biasMax = desc.biasMin = 0.005f;
					m_context->getEngine()->setShadowSource(desc);
				};
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

GMAsset Timeline::findAsset(const GMString& assetName)
{
	auto assetIter = m_assets.find(assetName);
	if (assetIter != m_assets.end())
		return assetIter->second;

	return GMAsset::invalidAsset();
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

void Timeline::bindAction(const Action& a)
{
	m_actions.insert(a);
}

void Timeline::runImmediateActions()
{
	for (const auto& action : m_actions)
	{
		if (action.runType == Action::Immediate)
		{
			action.action();
		}
	}
}

void Timeline::runActions()
{
	while (m_currentAction != m_actions.end())
	{
		if (m_currentAction->runType == Action::Deferred)
		{
			auto next = m_currentAction;
			++next;
			if (next == m_actions.end())
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
		}

		++m_currentAction;
	}
	
	if (m_currentAction == m_actions.end())
	{
		m_finished = true;
		m_playing = false;
	}
}
