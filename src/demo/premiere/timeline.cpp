#include "stdafx.h"
#include "timeline.h"
#include <gmimagebuffer.h>
#include <gmimage.h>
#include <gmgameobject.h>
#include <gmlight.h>

#define NoComponent 0
#define PositionComponent 0x01
#define DirectionComponent 0x02

using namespace tinyxml2;

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
	tinyxml2::XMLDocument doc;
	std::string content = timelineContent.toStdString();
	if (XML_SUCCESS == doc.Parse(content.c_str()))
	{
		auto root = doc.RootElement();
		if (GMString::stringEquals(root->Name(), "timeline"))
		{
			auto firstElement = root->FirstChildElement();
			parseElements(firstElement);
			return true;
		}
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

void Timeline::parseElements(tinyxml2::XMLElement* e)
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

void Timeline::parseAssets(tinyxml2::XMLElement* e)
{
	auto package = GM.getGamePackageManager();
	while (e)
	{
		GMString id = e->Attribute("id");
		GMString name = e->Name();
		if (name ==  L"cubemap")
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

		e = e->NextSiblingElement();
	}
}

void Timeline::parseObjects(tinyxml2::XMLElement* e)
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
			GMString asset = e->Attribute("asset");
			auto assetIter = m_assets.find(asset);
			if (assetIter != m_assets.end())
				obj = m_objects[id] = new GMCubeMapGameObject(assetIter->second);
			else
				gm_warning(gm_dbg_wrap("Cannot find asset: {0}"), asset);

			parseTransform(obj, e);
		}

		e = e->NextSiblingElement();
	}
}

void Timeline::parseActions(tinyxml2::XMLElement* e)
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
				GMString dirStr, posStr;
				GMVec3 direction, position;
				dirStr = e->Attribute("direction");
				posStr = e->Attribute("position");
				if (!dirStr.isEmpty() && !posStr.isEmpty())
				{
					{
						GMfloat x = 0, y = 0, z = 0;
						GMScanner scanner(dirStr);
						scanner.nextFloat(x);
						scanner.nextFloat(y);
						scanner.nextFloat(z);
						direction = GMVec3(x, y, z);
					}

					{
						GMfloat x = 0, y = 0, z = 0;
						GMScanner scanner(posStr);
						scanner.nextFloat(x);
						scanner.nextFloat(y);
						scanner.nextFloat(z);
						position = GMVec3(x, y, z);
					}

					action.action = [this, direction, position]() {
						auto& camera = m_context->getEngine()->getCamera();
						GMCameraLookAt lookAt(direction, position);
						camera.lookAt(lookAt);
					};
					
					bindAction(action);
				}
			}
			else if (type == L"addObject")
			{
				GMString id = e->Attribute("id");
				auto objectIter = m_objects.find(id);
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
					gm_warning(gm_dbg_wrap("Cannot find object: {0}"), id);
				}
			}
			else if (type == L"addLight")
			{
				GMString id = e->Attribute("id");
				auto lightIter = m_lights.find(id);
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
					gm_warning(gm_dbg_wrap("Cannot find light: {0}"), id);
				}
			}
			else if (type == L"lerp")
			{
				if (!time.isEmpty())
				{
					GMint32 component = NoComponent;

					GMfloat t = GMString::parseFloat(time);
					action.timePoint = t;

					GMString id = e->Attribute("id");
					if (id == L"$camera")
					{
						GMVec3 pos, dir;
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

						if (component != NoComponent)
						{
							action.runType = Action::Immediate; // lerp动作的添加是立即的
							action.action = [this, component, pos, dir, t]() {
								GMAnimation& animation = m_animations[Camera];
								GMCamera& camera = m_context->getEngine()->getCamera();
								const GMCameraLookAt& lookAt = camera.getLookAt();
								animation.setTargetObjects(&camera);

								GMVec3 posCandidate = (component & PositionComponent) ? pos : lookAt.position;
								GMVec3 dirCandidate = (component & DirectionComponent) ? dir : lookAt.lookDirection;
								animation.addKeyFrame(new GMCameraKeyframe(posCandidate, dirCandidate, t));
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

void Timeline::parseTransform(GMGameObject* o, tinyxml2::XMLElement* e)
{
	GM_ASSERT(o);
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
