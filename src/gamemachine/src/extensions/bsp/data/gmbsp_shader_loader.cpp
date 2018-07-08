#include "stdafx.h"
#include "gmbsp_shader_loader.h"
#include "foundation/vector.h"
#include <string>
#include "foundation/utilities/tools.h"
#include "foundation/utilities/tinyxml/tinyxml.h"
#include "extensions/bsp/render/gmbspgameworld.h"
#include "gmdata/imagereader/gmimagereader.h"
#include "gmdata/gamepackage/gmgamepackage.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "foundation/gamemachine.h"

#define BEGIN_PARSE(name) if ( GMString::stringEquals(it->Value(), #name) ) parse_##name(shader, it)
#define PARSE(name) else if ( GMString::stringEquals(it->Value(), #name) ) parse_##name(shader, it)
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

namespace
{
	GMuint parseSurfaceParm(const char* p)
	{
		for (GMuint i = 0; i < GMS_SURFACE_FLAG_MAX; i++)
		{
			if (GMString::stringEquals(p, _surface_flags[i].name))
				return _surface_flags[i].flag;
		}

		GM_ASSERT(false);
		gm_error("wrong surfaceparm {0}", { p });
		return 0;
	}

	GMS_BlendFunc parseBlendFunc(const GMString& p)
	{
		if (p == L"GMS_ZERO")
			return GMS_BlendFunc::ZERO;

		if (p == L"GMS_ONE")
			return GMS_BlendFunc::ONE;

		if (p == L"GMS_DST_COLOR")
			return GMS_BlendFunc::ONE;

		gm_warning("Unknown blendFunc {0} treated as GMS_ZERO", { p });
		return GMS_BlendFunc::ZERO;
	}

	void loadImage(const GMString& filename, const GMBuffer* buf, OUT GMImage** image)
	{
		if (GMImageReader::load(buf->buffer, buf->size, image))
			gm_info(L"loaded texture {0} from shader", { filename });
		else
			gm_error(L"texture {0} not found", { filename });
	}

	void readTernaryFloatsFromString(const char* str, GMfloat vec[3])
	{
		GMScanner s(str);
		GMfloat f;
		for (GMint i = 0; i < 3; i++)
		{
			s.nextFloat(&f);
			vec[i] = f;
		}
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
		GM_delete(*iter);
	}
}

void GMBSPShaderLoader::init(const GMString& directory, GMBSPGameWorld* world, GMBSPRenderData* bspRender)
{
	D(d);
	d->directory = directory;
	d->world = world;
	d->bspRender = bspRender;
}

ITexture* GMBSPShaderLoader::addTextureToTextureContainer(const GMString& name)
{
	D(d);
	GMAssets& assets = d->world->getAssets();
	GMAssetsNode* texNode = assets.getNodeFromPath(GM_ASSET_TEXTURES);
	GM_ASSERT(texNode);
	GMAssetsNode* node = GMAssets::findChild(texNode, name);
	if (!node)
	{
		GMString fn;
		GMBuffer buf;
		if (!GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Textures, name, &buf, &fn))
		{
			gm_warning(L"file {0} not found.", { fn });
			return nullptr;
		}

		GMImage* img = nullptr;
		loadImage(fn.toStdWString().c_str(), &buf, &img);

		if (img)
		{
			ITexture* texture = nullptr;
			IFactory* factory = GameMachine::instance().getFactory();
			factory->createTexture(d->world->getContext(), img, &texture);
			delete img;

			GM_ASSERT(texture);
			assets.insertAsset(GM_ASSET_TEXTURES, GMString(name), GMAssetType::Texture, texture);
			return texture;
		}
		return nullptr;
	}
	else
	{
		return GMAssets::getTexture(node->asset);
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

bool GMBSPShaderLoader::findItem(const GMString& name, GMint lightmapId, REF GMShader* shader)
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
		gm_error("xml load error at {0}: {1}", { GMString(doc->ErrorRow()), doc->ErrorDesc() });
		delete doc;
		return;
	}

	d->shaderDocs.push_back(doc);
	TiXmlElement* root = doc->RootElement();
	TiXmlElement* it = root->FirstChildElement();
	for (; it; it = it->NextSiblingElement())
	{
		TiXmlElement* elem = it;
		if (!GMString::stringEquals(elem->Value(), "item"))
			gm_warning(L"First node must be 'item'.");

		const char* name = elem->Attribute("name");
		const char* ref = elem->Attribute("ref");
		// 使用ref，可以引用另外一个item
		if (ref)
		{
			for (TiXmlElement* it = root->FirstChildElement(); it; it = it->NextSiblingElement())
			{
				if (GMString::stringEquals(ref, it->Attribute("name")))
				{
					elem = it;
					break;
				}
			}
		}

		d->items.insert(std::make_pair(name, elem));
	}
}

void GMBSPShaderLoader::parseItem(TiXmlElement* elem, GMint lightmapId, REF GMShader* shaderPtr)
{
	D(d);
	if (!shaderPtr)
		return;

	parseStart();
	// 这个是用于追踪from=lightmap的纹理使用的
	d->lightmapId = lightmapId;

	GMShader& shader = *shaderPtr;
	shader.getMaterial().ks = shader.getMaterial().kd = GMVec3(0);
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
}

void GMBSPShaderLoader::parseEnd()
{
	D(d);
	d->lightmapId = -1;
}

void GMBSPShaderLoader::parse_surfaceparm(GMShader& shader, TiXmlElement* elem)
{
	shader.setSurfaceFlag(shader.getSurfaceFlag() | parseSurfaceParm(elem->GetText()));
}

void GMBSPShaderLoader::parse_cull(GMShader& shader, TiXmlElement* elem)
{
	const char* text = elem->GetText();
	if (GMString::stringEquals(text, "none"))
		shader.setCull(GMS_Cull::NONE);
	else if (GMString::stringEquals(text, "cull"))
		shader.setCull(GMS_Cull::CULL);
	else
		gm_error("wrong cull param {0}", { text });
}

void GMBSPShaderLoader::parse_blendFunc(GMShader& shader, TiXmlElement* elem)
{
	const char* b = elem->GetText();
	if (b)
	{
		GMScanner s(b);
		GMString blendFunc;
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

void GMBSPShaderLoader::parse_animMap(GMShader& shader, TiXmlElement* elem)
{
	D(d);
	GMTextureSampler* sampler = &shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	GMint ms = GMString::parseInt(elem->Attribute("ms"));
	sampler->setAnimationMs(ms);

	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement())
	{
		BEGIN_PARSE(src);
		END_PARSE;
	}
	parse_map_tcMod(shader, elem);
}

void GMBSPShaderLoader::parse_src(GMShader& shader, TiXmlElement* elem)
{
	D(d);
	GMTextureSampler* sampler = &shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	ITexture* texture = addTextureToTextureContainer(elem->GetText());
	if (texture)
		sampler->addFrame(texture);
}

void GMBSPShaderLoader::parse_clampmap(GMShader& shader, TiXmlElement* elem)
{
	D(d);
	GMTextureSampler* sampler = &shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	ITexture* texture = addTextureToTextureContainer(elem->GetText());
	if (texture)
	{
		// TODO: GL_CLAMP
		sampler->setWrapS(GMS_Wrap::MIRRORED_REPEAT);
		sampler->setWrapT(GMS_Wrap::MIRRORED_REPEAT);
		sampler->addFrame(texture);
		parse_map_tcMod(shader, elem);
	}
	else
	{
		parse_map_fromLightmap(shader, elem);
	}
}

void GMBSPShaderLoader::parse_map(GMShader& shader, TiXmlElement* elem)
{
	D(d);
	GMTextureSampler* sampler = &shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	ITexture* texture = addTextureToTextureContainer(elem->GetText());
	if (texture)
	{
		sampler->setWrapS(GMS_Wrap::REPEAT);
		sampler->setWrapT(GMS_Wrap::REPEAT);
		sampler->addFrame(texture);
		parse_map_tcMod(shader, elem);
	}
	else
	{
		parse_map_fromLightmap(shader, elem);
	}
}

void GMBSPShaderLoader::parse_map_fromLightmap(GMShader& shader, TiXmlElement* elem)
{
	D(d);
	// from "lightmap"
	const char* from = elem->Attribute("from");
	if (from)
	{
		if (GMString::stringEquals(from, "lightmap"))
		{
			GMAssets& assets = d->world->getAssets();
			GMAssetsNode* lightmapNode = assets.getNodeFromPath(GM_ASSET_LIGHTMAPS);

			GMTextureSampler* sampler = &shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
			GMAssetsNode* node = GMAssets::findChild(lightmapNode, std::to_string(d->lightmapId).c_str());
			GM_ASSERT(node);

			ITexture* tex = GMAssets::getTexture(node->asset);
			if (tex)
			{
				sampler->addFrame(tex);
				gm_info(L"found map from lightmap {0}", { GMString(d->lightmapId) });
			}
			else
			{
				gm_error(L"lightmap not found: {1}", { GMString(d->lightmapId) });
			}
		}
	}
}

void GMBSPShaderLoader::parse_normalmap(GMShader& shader, TiXmlElement* elem)
{
	GMTextureSampler* sampler = &shader.getTextureList().getTextureSampler(GMTextureType::NormalMap);
	ITexture* texture = addTextureToTextureContainer(elem->GetText());
	if (texture)
	{
		sampler->setWrapS(GMS_Wrap::REPEAT);
		sampler->setWrapT(GMS_Wrap::REPEAT);
		sampler->addFrame(texture);
	}
}

void GMBSPShaderLoader::parse_lights(GMShader& shader, TiXmlElement* elem)
{
	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement())
	{
		BEGIN_PARSE(light);
		END_PARSE;
	}
}

void GMBSPShaderLoader::parse_light(GMShader& shader, TiXmlElement* elem)
{
	D(d);
	const char* type = elem->Attribute("type");
	if (!type)
	{
		gm_error(L"light type missing.");
		return;
	}

	GMMaterial& material = shader.getMaterial();
	IFactory* factory = GM.getFactory();
	ILight* light = nullptr;;
	const char* color = elem->Attribute("color");
	if (!color)
	{
		gm_error(L"light color missing.");
		return;
	}

	if (GMString::stringEquals(type, "ambient"))
	{
		factory->createLight(GMLightType::Ambient, &light);
	}
	else if (GMString::stringEquals(type, "specular"))
	{
		factory->createLight(GMLightType::Direct, &light);
		const char* position = elem->Attribute("position");
		if (!position)
		{
			gm_error(L"specular light position missing.");
			return;
		}
		GMScanner s(color);
		GMfloat vecPosition[3];
		readTernaryFloatsFromString(position, vecPosition);
		light->setLightPosition(&vecPosition[0]);

		GMfloat arg[3];
		const char* k = elem->Attribute("ks");
		if (!k)
		{
			readTernaryFloatsFromString("0 0 0", arg);
			material.ks = MakeVector3(arg);
		}
		else
		{
			readTernaryFloatsFromString(k, arg);
			material.ks = MakeVector3(arg);
		}

		k = elem->Attribute("kd");
		if (!k)
		{
			readTernaryFloatsFromString("0 0 0", arg);
			material.ks = MakeVector3(arg);
		}
		else
		{
			readTernaryFloatsFromString(k, arg);
			material.ks = MakeVector3(arg);
		}

		k = elem->Attribute("shininess");
		GMfloat shininess = 0;
		if (!k)
		{
			material.shininess = shininess;
		}
		else
		{
			shininess = GMString::parseFloat(k);
			material.shininess = shininess;
		}
	}

	GM_ASSERT(light);
	GMfloat vecColor[3];
	readTernaryFloatsFromString(color, vecColor);
	light->setLightColor(&vecColor[0]);

	d->world->getContext()->getEngine()->addLight(light);
}

void GMBSPShaderLoader::parse_map_tcMod(GMShader& shader, TiXmlElement* elem)
{
	D(d);
	// tcMod <type> <...>
	const char* tcMod = elem->Attribute("tcMod");
	GMuint tcModNum = 0;
	while (tcModNum < MAX_TEX_MOD && shader.getTextureList().getTextureSampler(GMTextureType::Ambient).getTexMod(tcModNum).type != GMS_TextureModType::NO_TEXTURE_MOD)
	{
		tcModNum++;
	}
	GMS_TextureMod* currentMod = &shader.getTextureList().getTextureSampler(GMTextureType::Ambient).getTexMod(tcModNum);

	if (tcMod)
	{
		GMScanner s(tcMod);
		GMString type;
		s.next(type);

		while (true)
		{
			if (type == L"scroll")
			{
				currentMod->type = GMS_TextureModType::SCROLL;
				s.nextFloat(&currentMod->p1);
				s.nextFloat(&currentMod->p2);
			}
			else if (type == L"scale")
			{
				currentMod->type = GMS_TextureModType::SCALE;
				s.nextFloat(&currentMod->p1);
				s.nextFloat(&currentMod->p2);
			}
			
			s.next(type);
			if (type.isEmpty())
				break;
			tcModNum++;
			if (tcModNum == MAX_TEX_MOD)
			{
				if (!type.isEmpty())
					gm_warning(L"warning: you have tcMods more than {0}, please increase MAX_TEX_MOD", { GMString(MAX_TEX_MOD) });
				break;
			}
			currentMod = &shader.getTextureList().getTextureSampler(GMTextureType::Ambient).getTexMod(tcModNum);
		}
	}
}

void GMBSPShaderLoader::createSky(GMShader& shader)
{
	D(d);
	ITexture* texture = shader.getTextureList().getTextureSampler(GMTextureType::Ambient).getFrameByIndex(0);
	shader.setDiscard(true);
	if (!d->world->getSky())
	{
		GMShader skyShader = shader;
		skyShader.setDiscard(false);
		skyShader.setCull(GMS_Cull::NONE);

		GMBSPSkyGameObject* sky = new GMBSPSkyGameObject(skyShader, MakeVector3(d->bspRender->boundMin), MakeVector3(d->bspRender->boundMax));
		d->world->setSky(sky);
	}
}