#include "stdafx.h"
#include "bsp_shader_loader.h"
#include <vector>
#include <string>
#include "utilities/path.h"
#include "utilities/tinyxml/tinyxml.h"
#include "gmdatacore/shader.h"
#include "gmengine/elements/bspgameworld.h"
#include "utilities/scanner.h"
#include "gmdatacore/imagereader/imagereader.h"
#include "gmengine/controllers/factory.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmengine/elements/skygameobject.h"

#define BEGIN_PARSE(name) if ( strEqual(it->Value(), #name) ) parse_##name(shader, it)
#define BEGIN_PARSE_I(name, i) if ( strEqual(it->Value(), #name) ) parse_##name(shader, it, i)
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

	ASSERT(false);
	gm_error("wrong surfaceparm %s", p);
	return 0;
}

static GMS_BlendFunc parseBlendFunc(const char* p)
{
	if (strEqual(p, "GMS_ZERO"))
		return GMS_ZERO;

	if (strEqual(p, "GMS_ONE"))
		return GMS_ONE;

	if (strEqual(p, "GMS_DST_COLOR"))
		return GMS_ONE;

	gm_warning("Unknown blendFunc %s treated as GMS_ZERO", p);
	return GMS_ZERO;
}

static void loadImage(const char* filename, OUT Image** image)
{
	if (ImageReader::load(filename, image))
		gm_info("loaded texture %s from shader", filename);
	else
		gm_error("texture %s not found", filename);
}

BSPShaderLoader::BSPShaderLoader()
	: m_world(nullptr)
	, m_bspRender(nullptr)
{
}

BSPShaderLoader::~BSPShaderLoader()
{
	for (auto iter = m_shaderDocs.begin(); iter != m_shaderDocs.end(); iter++)
	{
		delete *iter;
	}
}

void BSPShaderLoader::init(const char* directory, BSPGameWorld* world, BSPRenderData* bspRender)
{
	m_directory = directory;
	m_world = world;
	m_bspRender = bspRender;
}

ITexture* BSPShaderLoader::addTextureToWorld(Shader& shader, const char* name)
{
	if (!name)
		return nullptr;

	ResourceContainer* rc = m_world->getGraphicEngine()->getResourceContainer();
	TextureContainer& tc = rc->getTextureContainer();
	const TextureContainer::TextureItem* item = tc.find(name);
	if (!item)
	{
		std::string fn = m_world->bspWorkingDirectory();
		fn.append(name);
		Image* img = nullptr;
		loadImage(fn.c_str(), &img);

		if (img)
		{
			ITexture* texture;
			IFactory* factory = m_world->getGameMachine()->getFactory();
			factory->createTexture(img, &texture);

			TextureContainer::TextureItem ti;
			ti.name = name;
			ti.texture = texture;
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

void BSPShaderLoader::load()
{
	std::vector<std::string> files = Path::getAllFiles(m_directory.c_str());
	// load all item tag, but not parse them until item is needed
	for (auto iter = files.begin(); iter != files.end(); iter++)
	{
		parse((*iter).c_str());
	}
}

bool BSPShaderLoader::findItem(const char* name, GMuint lightmapId, REF Shader* shader)
{
	auto foundResult = m_items.find(name);
	if (foundResult == m_items.end())
		return false;

	// If we found it, parse it.
	parseItem((*foundResult).second, lightmapId, shader);
	return true;
}

void BSPShaderLoader::parse(const char* filename)
{
	TiXmlDocument* doc = new TiXmlDocument();

	if (!doc->LoadFile(filename))
	{
		gm_error("xml load error at %d: %s", doc->ErrorRow(), doc->ErrorDesc());
		delete doc;
		return;
	}

	m_shaderDocs.push_back(doc);
	TiXmlElement* root = doc->RootElement();
	TiXmlElement* it = root->FirstChildElement();
	for (; it; it = it->NextSiblingElement())
	{
		TiXmlElement* elem = it;
		if (!strEqual(elem->Value(), "item"))
			gm_warning("First node must be 'item'.");

		Shader shader;
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

		m_items.insert(std::make_pair(name, elem));
	}
}

void BSPShaderLoader::parseItem(TiXmlElement* elem, GMuint lightmapId, REF Shader* shaderPtr)
{
	if (!shaderPtr)
		return;

	parseStart();
	// 这个是用于追踪from=lightmap的纹理使用的
	m_lightmapId = lightmapId;

	Shader& shader = *shaderPtr;
	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement())
	{
		BEGIN_PARSE(surfaceparm); // surfaceparm一定要在最先
		PARSE(cull);
		PARSE(blendFunc);
		PARSE(animMap);
		PARSE(clampmap);
		PARSE(map);
		END_PARSE;
	}

	if (shader.surfaceFlag & SURF_SKY)
		createSky(shader);

	parseEnd();
}

void BSPShaderLoader::parseStart()
{
	m_textureNum = 0;
}

void BSPShaderLoader::parseEnd()
{
	m_lightmapId = -1;
}

void BSPShaderLoader::parse_surfaceparm(Shader& shader, TiXmlElement* elem)
{
	shader.surfaceFlag |= parseSurfaceParm(elem->GetText());;
}

void BSPShaderLoader::parse_cull(Shader& shader, TiXmlElement* elem)
{
	const char* text = elem->GetText();
	if (strEqual(text, "none"))
		shader.cull = GMS_NONE;
	else if (strEqual(text, "cull"))
		shader.cull = GMS_CULL;
	else
		gm_error("wrong cull param %s", text);
}

void BSPShaderLoader::parse_blendFunc(Shader& shader, TiXmlElement* elem)
{
	const char* b = elem->GetText();
	if (b)
	{
		Scanner s(b);
		char blendFunc[LINE_MAX];
		s.next(blendFunc);
		shader.blendFactors[0] = parseBlendFunc(blendFunc);
		s.next(blendFunc);
		shader.blendFactors[1] = parseBlendFunc(blendFunc);
		shader.blend = true;
	}
	else
	{
		shader.blend = false;
	}
}

void BSPShaderLoader::parse_animMap(Shader& shader, TiXmlElement* elem)
{
	TextureFrames* frame = &shader.texture.textures[m_textureNum];
	GMint ms;
	SAFE_SSCANF(elem->Attribute("ms"), "%d", &ms);
	frame->animationMs = ms;

	GMuint frameCount = 0;
	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement(), frameCount++)
	{
		BEGIN_PARSE_I(src, frameCount);
		END_PARSE;
	}
	frame->frameCount = frameCount;
	parse_map_tcMod(shader, elem);
	m_textureNum++;
}

void BSPShaderLoader::parse_src(Shader& shader, TiXmlElement* elem, GMuint i)
{
	TextureFrames* frame = &shader.texture.textures[m_textureNum];
	ITexture* texture = addTextureToWorld(shader, elem->GetText());
	if (texture)
		frame->frames[i] = texture;
}

void BSPShaderLoader::parse_clampmap(Shader& shader, TiXmlElement* elem)
{
	TextureFrames* frame = &shader.texture.textures[m_textureNum];
	ITexture* texture = addTextureToWorld(shader, elem->GetText());
	if (texture)
	{
		// TODO: GL_CLAMP
		frame->wrapS = GMS_MIRRORED_REPEAT;
		frame->wrapT = GMS_MIRRORED_REPEAT;
		frame->frames[0] = texture;
		frame->frameCount = 1;
		parse_map_tcMod(shader, elem);
		m_textureNum++;
	}
	else
	{
		parse_map_fromLightmap(shader, elem);
	}
}

void BSPShaderLoader::parse_map(Shader& shader, TiXmlElement* elem)
{
	TextureFrames* frame = &shader.texture.textures[m_textureNum];
	ITexture* texture = addTextureToWorld(shader, elem->GetText());
	if (texture)
	{
		frame->wrapS = GMS_REPEAT;
		frame->wrapT = GMS_REPEAT;
		frame->frames[0] = texture;
		frame->frameCount = 1;
		parse_map_tcMod(shader, elem);
		m_textureNum++;
	}
	else
	{
		parse_map_fromLightmap(shader, elem);
	}
}

void BSPShaderLoader::parse_map_fromLightmap(Shader& shader, TiXmlElement* elem)
{
	// from "lightmap"
	const char* from = elem->Attribute("from");
	if (from)
	{
		if (strEqual(from, "lightmap"))
		{
			ResourceContainer* rc = m_world->getGameMachine()->getGraphicEngine()->getResourceContainer();
			TextureContainer_ID& tc = rc->getLightmapContainer();

			TextureFrames* frame = &shader.texture.textures[m_textureNum];
			memset(frame->frames, 0, sizeof(frame->frames));
			const TextureContainer_ID::TextureItem* tex = tc.find(m_lightmapId);
			if (tex)
			{
				frame->frames[0] = tc.find(m_lightmapId)->texture;
				frame->frameCount = 1;
				gm_info("found map from lightmap %d", m_lightmapId);
			}
			else
			{
				gm_error("lightmap not found: %d", m_lightmapId);
			}
		}
	}
}

void BSPShaderLoader::parse_map_tcMod(Shader& shader, TiXmlElement* elem)
{
	// tcMod <type> <...>
	const char* tcMod = elem->Attribute("tcMod");
	GMuint tcModNum = 0;
	while (tcModNum < MAX_TEX_MOD && shader.texture.textures[m_textureNum].texMod[tcModNum].type != GMS_NO_TEXTURE_MOD)
	{
		tcModNum++;
	}
	GMS_TextureMod* currentMod = &shader.texture.textures[m_textureNum].texMod[tcModNum];

	if (tcMod)
	{
		Scanner s(tcMod);
		char type[LINE_MAX];
		s.next(type);

		while (true)
		{
			if (strEqual(type, "scroll"))
			{
				currentMod->type = GMS_SCROLL;
				s.nextFloat(&currentMod->p1);
				s.nextFloat(&currentMod->p2);
			}
			else if (strEqual(type, "scale"))
			{
				currentMod->type = GMS_SCALE;
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
					gm_warning("warning: you have tcMods more than %d, please increase MAX_TEX_MOD", MAX_TEX_MOD);
				break;
			}
			currentMod = &shader.texture.textures[m_textureNum].texMod[tcModNum];
		}
	}
}

void BSPShaderLoader::createSky(Shader& shader)
{
	ITexture* texture = shader.texture.textures[TEXTURE_INDEX_AMBIENT].frames[0];
	shader.nodraw = true;
	if (!m_world->getSky())
	{
		Shader skyShader = shader;
		skyShader.nodraw = false;
		skyShader.cull = GMS_NONE;
		skyShader.noDepthTest = true;

		SkyGameObject* sky = new SkyGameObject(skyShader, m_bspRender->boundMin, m_bspRender->boundMax);
		m_world->setSky(sky);
	}
}