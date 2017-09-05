#include "stdafx.h"
#include "gmbsp_shader_loader.h"
#include "foundation/vector.h"
#include <string>
#include "foundation/utilities/utilities.h"
#include "foundation/utilities/tinyxml/tinyxml.h"
#include <shader.h>
#include "gmengine/gmbspgameworld.h"
#include "gmdatacore/imagereader/gmimagereader.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"
#include "gmengine/gmgameobject.h"
#include "foundation/gamemachine.h"

#define BEGIN_PARSE(name) if ( strEqual(it->Value(), #name) ) parse_##name(shader, it)
#define PARSE(name) else if ( strEqual(it->Value(), #name) ) parse_##name(shader, it)
#define END_PARSE

enum
{
	GMS_SURFACE_FLAG_MAX = 19
};

enum
{
	SKY_SUBDIVISIONS = 8,
	HALF_SKY_SUBDIVISIONS = SKY_SUBDIVISIONS / 2,
};

struct SurfaceFlags
{
	const char* name;
	GMuint flag;
} _surface_flags[GMS_SURFACE_FLAG_MAX] = {
	{ "nodamage", SURF_NODAMAGE },
	{ "slick", SURF_SLICK },
	{ "sky", SURF_SKY },
	{ "ladder", SURF_LADDER },
	{ "noimpact", SURF_NOIMPACT },
	{ "nomarks", SURF_NOMARKS },
	{ "flesh", SURF_FLESH },
	{ "nodraw", SURF_NODRAW },
	{ "hint", SURF_HINT },
	{ "skip", SURF_SKIP },
	{ "nolightmap", SURF_NOLIGHTMAP },
	{ "pointlight", SURF_POINTLIGHT },
	{ "metalsteps", SURF_METALSTEPS },
	{ "nosteps", SURF_NOSTEPS },
	{ "nonsolid", SURF_NONSOLID },
	{ "lightfilter", SURF_LIGHTFILTER },
	{ "alphashadow", SURF_ALPHASHADOW },
	{ "nodlight", SURF_NODLIGHT },
	{ "dust", SURF_DUST },
};

static GMuint parseSurfaceParm(const char* p)
{
	for (GMuint i = 0; i < GMS_SURFACE_FLAG_MAX; i++)
	{
		if (strEqual(p, _surface_flags[i].name))
			return _surface_flags[i].flag;
	}

	GM_ASSERT(false);
	gm_error("wrong surfaceparm %s", p);
	return 0;
}

static GMS_BlendFunc parseBlendFunc(const char* p)
{
	if (strEqual(p, "GMS_ZERO"))
		return GMS_BlendFunc::ZERO;

	if (strEqual(p, "GMS_ONE"))
		return GMS_BlendFunc::ONE;

	if (strEqual(p, "GMS_DST_COLOR"))
		return GMS_BlendFunc::ONE;

	gm_warning("Unknown blendFunc %s treated as GMS_ZERO", p);
	return GMS_BlendFunc::ZERO;
}

static void loadImage(const GMString& filename, const GMBuffer* buf, OUT GMImage** image)
{
	if (GMImageReader::load(buf->buffer, buf->size, image))
		gm_info(_L("loaded texture %Ls from shader"), filename.toStdWString().c_str());
	else
		gm_error(_L("texture %Ls not found"), filename.toStdWString().c_str());
}

static void readTernaryFloatsFromString(const char* str, linear_math::Vector3& vec)
{
	Scanner s(str);
	for (GMint i = 0; i < 3; i++)
	{
		GMfloat f;
		s.nextFloat(&f);
		vec[i] = f;
	}
}

GMBSPShaderLoader::GMBSPShaderLoader()
{
	D(d);
	d->world = nullptr;
	d->bspRender = nullptr;
}

GMBSPShaderLoader::~GMBSPShaderLoader()
{
	D(d);
	for (auto iter = d->shaderDocs.begin(); iter != d->shaderDocs.end(); iter++)
	{
		delete *iter;
	}
}

void GMBSPShaderLoader::init(const GMString& directory, GMBSPGameWorld* world, GMBSPRenderData* bspRender)
{
	D(d);
	d->directory = directory;
	d->world = world;
	d->bspRender = bspRender;
}

ITexture* GMBSPShaderLoader::addTextureToTextureContainer(const char* name)
{
	D(d);
	if (!name)
		return nullptr;

	GMResourceContainer& rc = d->world->getResourceContainer();
	GMTextureContainer& tc = rc.getTextureContainer();
	const GMTextureContainer::TextureItemType* item = tc.find(name);
	if (!item)
	{
		GMString fn;
		GMBuffer buf;
		if (!GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Textures, name, &buf, &fn))
		{
			gm_warning(_L("file %Ls not found."), fn.toStdWString().c_str());
			return nullptr;
		}

		GMImage* img = nullptr;
		loadImage(fn.toStdWString().c_str(), &buf, &img);

		if (img)
		{
			ITexture* texture;
			IFactory* factory = GameMachine::instance().getFactory();
			factory->createTexture(img, &texture);

			GMTextureContainer::TextureItemType ti = { name, texture };
			tc.insert(ti);
			return texture;
		}
		return nullptr;
	}
	else
	{
		return item->texture;
	}
}

void GMBSPShaderLoader::load()
{
	D(d);
	GMGamePackage* pk = GameMachine::instance().getGamePackageManager();
	Vector<GMString> files = pk->getAllFiles(d->directory);

	// load all item tag, but not parse them until item is needed
	for (auto& file : files)
	{
		GMBuffer buf;
		pk->readFileFromPath(file, &buf);
		buf.convertToStringBuffer();
		parse((const char*) buf.buffer);
	}
}

bool GMBSPShaderLoader::findItem(const GMString& name, GMint lightmapId, REF Shader* shader)
{
	D(d);
	auto foundResult = d->items.find(name);
	if (foundResult == d->items.end())
		return false;

	// If we found it, parse it.
	parseItem((*foundResult).second, lightmapId, shader);
	return true;
}

void GMBSPShaderLoader::parse(const char* buffer)
{
	D(d);
	TiXmlDocument* doc = new TiXmlDocument();
	if (doc->Parse(buffer) != 0)
	{
		gm_error("xml load error at %d: %s", doc->ErrorRow(), doc->ErrorDesc());
		delete doc;
		return;
	}

	d->shaderDocs.push_back(doc);
	TiXmlElement* root = doc->RootElement();
	TiXmlElement* it = root->FirstChildElement();
	for (; it; it = it->NextSiblingElement())
	{
		TiXmlElement* elem = it;
		if (!strEqual(elem->Value(), "item"))
			gm_warning(_L("First node must be 'item'."));

		const char* name = elem->Attribute("name");
		const char* ref = elem->Attribute("ref");
		// 使用ref，可以引用另外一个item
		if (ref)
		{
			for (TiXmlElement* it = root->FirstChildElement(); it; it = it->NextSiblingElement())
			{
				if (strEqual(ref, it->Attribute("name")))
				{
					elem = it;
					break;
				}
			}
		}

		d->items.insert(std::make_pair(name, elem));
	}
}

void GMBSPShaderLoader::parseItem(TiXmlElement* elem, GMint lightmapId, REF Shader* shaderPtr)
{
	D(d);
	if (!shaderPtr)
		return;

	parseStart();
	// 这个是用于追踪from=lightmap的纹理使用的
	d->lightmapId = lightmapId;

	Shader& shader = *shaderPtr;
	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement())
	{
		BEGIN_PARSE(surfaceparm); // surfaceparm一定要在最先
		PARSE(cull);
		PARSE(blendFunc);
		PARSE(animMap);
		PARSE(clampmap);
		PARSE(map);
		PARSE(normalmap);
		PARSE(lights);
		END_PARSE;
	}

	if (shader.getSurfaceFlag() & SURF_SKY)
		createSky(shader);

	parseEnd();
}

void GMBSPShaderLoader::parseStart()
{
	D(d);
	d->textureNum = 0;
}

void GMBSPShaderLoader::parseEnd()
{
	D(d);
	d->lightmapId = -1;
}

void GMBSPShaderLoader::parse_surfaceparm(Shader& shader, TiXmlElement* elem)
{
	shader.setSurfaceFlag(shader.getSurfaceFlag() | parseSurfaceParm(elem->GetText()));
}

void GMBSPShaderLoader::parse_cull(Shader& shader, TiXmlElement* elem)
{
	const char* text = elem->GetText();
	if (strEqual(text, "none"))
		shader.setCull(GMS_Cull::NONE);
	else if (strEqual(text, "cull"))
		shader.setCull(GMS_Cull::CULL);
	else
		gm_error("wrong cull param %s", text);
}

void GMBSPShaderLoader::parse_blendFunc(Shader& shader, TiXmlElement* elem)
{
	const char* b = elem->GetText();
	if (b)
	{
		Scanner s(b);
		char blendFunc[LINE_MAX];
		s.next(blendFunc);
		shader.setBlendFactorSource(parseBlendFunc(blendFunc));
		s.next(blendFunc);
		shader.setBlendFactorDest(parseBlendFunc(blendFunc));
		shader.setBlend(true);
	}
	else
	{
		shader.setBlend(false);
	}
}

void GMBSPShaderLoader::parse_animMap(Shader& shader, TiXmlElement* elem)
{
	D(d);
	GMTextureFrames* frame = &shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, d->textureNum);
	GMint ms;
	SAFE_SSCANF(elem->Attribute("ms"), "%d", &ms);
	frame->setAnimationMs(ms);

	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement())
	{
		BEGIN_PARSE(src);
		END_PARSE;
	}
	parse_map_tcMod(shader, elem);
	d->textureNum++;
}

void GMBSPShaderLoader::parse_src(Shader& shader, TiXmlElement* elem)
{
	D(d);
	GMTextureFrames* frame = &shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, d->textureNum);
	ITexture* texture = addTextureToTextureContainer(elem->GetText());
	if (texture)
		frame->addFrame(texture);
}

void GMBSPShaderLoader::parse_clampmap(Shader& shader, TiXmlElement* elem)
{
	D(d);
	GMTextureFrames* frame = &shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, d->textureNum);
	ITexture* texture = addTextureToTextureContainer(elem->GetText());
	if (texture)
	{
		// TODO: GL_CLAMP
		frame->setWrapS(GMS_Wrap::MIRRORED_REPEAT);
		frame->setWrapT(GMS_Wrap::MIRRORED_REPEAT);
		frame->addFrame(texture);
		parse_map_tcMod(shader, elem);
		d->textureNum++;
	}
	else
	{
		parse_map_fromLightmap(shader, elem);
	}
}

void GMBSPShaderLoader::parse_map(Shader& shader, TiXmlElement* elem)
{
	D(d);
	GMTextureFrames* frame = &shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, d->textureNum);
	ITexture* texture = addTextureToTextureContainer(elem->GetText());
	if (texture)
	{
		frame->setWrapS(GMS_Wrap::REPEAT);
		frame->setWrapT(GMS_Wrap::REPEAT);
		frame->addFrame(texture);
		parse_map_tcMod(shader, elem);
		d->textureNum++;
	}
	else
	{
		parse_map_fromLightmap(shader, elem);
	}
}

void GMBSPShaderLoader::parse_map_fromLightmap(Shader& shader, TiXmlElement* elem)
{
	D(d);
	// from "lightmap"
	const char* from = elem->Attribute("from");
	if (from)
	{
		if (strEqual(from, "lightmap"))
		{
			GMResourceContainer& rc = d->world->getResourceContainer();
			GMTextureContainer_ID& tc = rc.getLightmapContainer();

			GMTextureFrames* frame = &shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, d->textureNum);
			const GMTextureContainer_ID::TextureItemType* tex = tc.find(d->lightmapId);
			if (tex)
			{
				frame->addFrame(tc.find(d->lightmapId)->texture);
				gm_info(_L("found map from lightmap %d"), d->lightmapId);
			}
			else
			{
				gm_error(_L("lightmap not found: %d"), d->lightmapId);
			}
		}
	}
}

void GMBSPShaderLoader::parse_normalmap(Shader& shader, TiXmlElement* elem)
{
	GMTextureFrames* frame = &shader.getTexture().getTextureFrames(GMTextureType::NORMALMAP, 0);
	ITexture* texture = addTextureToTextureContainer(elem->GetText());
	if (texture)
	{
		frame->setWrapS(GMS_Wrap::REPEAT);
		frame->setWrapT(GMS_Wrap::REPEAT);
		frame->addFrame(texture);
	}
}

void GMBSPShaderLoader::parse_lights(Shader& shader, TiXmlElement* elem)
{
	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement())
	{
		BEGIN_PARSE(light);
		END_PARSE;
	}
}

void GMBSPShaderLoader::parse_light(Shader& shader, TiXmlElement* elem)
{
	D(d);
	const char* type = elem->Attribute("type");
	if (!type)
	{
		gm_error(_L("light type missing."));
		return;
	}

	GMMaterial& material = shader.getMaterial();
	GMLight light(GMLightType::AMBIENT);
	GMLightType lightType;
	const char* color = elem->Attribute("color");
	if (!color)
	{
		gm_error(_L("light color missing."));
		return;
	}

	linear_math::Vector3 vecColor;
	readTernaryFloatsFromString(color, vecColor);
	light.setLightColor(&vecColor[0]);

	if (strEqual(type, "ambient"))
	{
		lightType = GMLightType::AMBIENT;
	}
	else if (strEqual(type, "specular"))
	{
		lightType = GMLightType::SPECULAR;
		const char* position = elem->Attribute("position");
		if (!position)
		{
			gm_error(_L("specular light position missing."));
			return;
		}
		Scanner s(color);
		linear_math::Vector3 vecPosition;
		readTernaryFloatsFromString(position, vecPosition);
		light.setLightPosition(&vecPosition[0]);

		linear_math::Vector3 arg;

		const char* k = elem->Attribute("ks");
		if (!k)
		{
			readTernaryFloatsFromString("1 1 1", arg);
			material.ks = arg;
		}
		else
		{
			readTernaryFloatsFromString(k, arg);
			material.ks = arg;
		}

		k = elem->Attribute("kd");
		if (!k)
		{
			readTernaryFloatsFromString("1 1 1", arg);
			material.ks = arg;
		}
		else
		{
			readTernaryFloatsFromString(k, arg);
			material.ks = arg;
		}

		k = elem->Attribute("shininess");
		GMfloat shininess = 0;
		if (!k)
		{
			material.shininess = shininess;
		}
		else
		{
			SAFE_SSCANF(k, "%f", &shininess);
			material.shininess = shininess;
		}
	}
	light.setType(lightType);
	d->world->addLight(light);
}

void GMBSPShaderLoader::parse_map_tcMod(Shader& shader, TiXmlElement* elem)
{
	D(d);
	// tcMod <type> <...>
	const char* tcMod = elem->Attribute("tcMod");
	GMuint tcModNum = 0;
	while (tcModNum < MAX_TEX_MOD && shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, d->textureNum).getTexMod(tcModNum).type != GMS_TextureModType::NO_TEXTURE_MOD)
	{
		tcModNum++;
	}
	GMS_TextureMod* currentMod = &shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, d->textureNum).getTexMod(tcModNum);

	if (tcMod)
	{
		Scanner s(tcMod);
		char type[LINE_MAX];
		s.next(type);

		while (true)
		{
			if (strEqual(type, "scroll"))
			{
				currentMod->type = GMS_TextureModType::SCROLL;
				s.nextFloat(&currentMod->p1);
				s.nextFloat(&currentMod->p2);
			}
			else if (strEqual(type, "scale"))
			{
				currentMod->type = GMS_TextureModType::SCALE;
				s.nextFloat(&currentMod->p1);
				s.nextFloat(&currentMod->p2);
			}
			
			s.next(type);
			if (!strlen(type))
				break;
			tcModNum++;
			if (tcModNum == MAX_TEX_MOD)
			{
				if (strlen(type))
					gm_warning(_L("warning: you have tcMods more than %d, please increase MAX_TEX_MOD"), MAX_TEX_MOD);
				break;
			}
			currentMod = &shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, d->textureNum).getTexMod(tcModNum);
		}
	}
}

void GMBSPShaderLoader::createSky(Shader& shader)
{
	D(d);
	ITexture* texture = shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, 0).getFrameByIndex(0);
	shader.setNodraw(true);
	if (!d->world->getSky())
	{
		Shader skyShader = shader;
		skyShader.setNodraw(false);
		skyShader.setCull(GMS_Cull::NONE);

		GMSkyGameObject* sky = new GMSkyGameObject(skyShader, d->bspRender->boundMin, d->bspRender->boundMax);
		d->world->setSky(sky);
	}
}