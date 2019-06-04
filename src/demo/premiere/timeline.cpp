#include "stdafx.h"
#include "timeline.h"
#include <gmimagebuffer.h>
#include <gmimage.h>
#include <gmgameobject.h>
#include <gmlight.h>

using namespace tinyxml2;

Timeline::Timeline(const IRenderContext* context, GMGameWorld* world)
	: m_context(context)
	, m_world(world)
{
	GM_ASSERT(m_context && m_world);
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

}

void Timeline::play()
{
	for (const auto& action : m_actions)
	{
		if (action.runType == Action::Immediate)
		{
			action.action();
		}
	}
}

void Timeline::pause()
{

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
			GMString asset = e->Attribute("asset");
			auto assetIter = m_assets.find(asset);
			if (assetIter != m_assets.end())
				m_objects[id] = new GMCubeMapGameObject(assetIter->second);
			else
				gm_warning(gm_dbg_wrap("Cannot find asset: {0}"), asset);
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
			Action action = { Action::Immediate, 0 };
			GMString time = e->Attribute("time");
			if (!time.isEmpty())
			{
				action.runType = Action::Deferred;
				action.timePoint = GMString::parseInt(time);
				if (action.timePoint == 0)
					action.runType = Action::Immediate;
			}

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
						camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);
					};
					m_actions.insert(action);
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
					m_actions.insert(action);
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
					m_actions.insert(action);
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
					m_actions.insert(action);
				}
				else
				{
					gm_warning(gm_dbg_wrap("Cannot find light: {0}"), id);
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
