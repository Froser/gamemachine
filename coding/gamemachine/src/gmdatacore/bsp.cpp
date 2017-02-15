#include "stdafx.h"
#include "bsp.h"
#include <stdio.h>
#include "utilities\assert.h"
#include "utilities\path.h"
#include "bsp_interior.inl"

BSP::BSP()
{
}

BSP::~BSP()
{
	D(d);
	free(d.buffer);
}

void BSP::loadBsp(const char* filename)
{
	D(d);
	d.filename = filename;
	readFile();
	swapBsp();
}

const BSPData& BSP::bspData()
{
	D(d);
	return d;
}

bool BSP::findVectorByName(float* outvec, const char* name)
{
	D(d);
	const char *cl;
	BSPVector3 origin;

	bool found = false;

	parseEntities();

	for (int i = 1; i < d.numentities; i++) {
		cl = valueForKey(&d.entities[i], "classname");
		if (!strcmp(cl, "info_player_start")) {
			vectorForKey(&d.entities[i], "origin", origin);
			found = true;
			break;
		}
		if (!strcmp(cl, "info_player_deathmatch")) {
			vectorForKey(&d.entities[i], "origin", origin);
			found = true;
			break;
		}
	}

	if (found)
	{
		outvec[0] = origin[0];
		outvec[1] = origin[1];
		outvec[2] = origin[2];
	}
	return found;
}


void BSP::readFile()
{
	D(d);
	FILE* file = nullptr;
	file = fopen(d.filename.c_str(), "r");
	if (file)
	{
		GMuint size;
		if (fseek(file, 0, SEEK_END) || (size = ftell(file)) == EOF || fseek(file, 0, SEEK_SET))
		{
			LOG_ASSERT_MSG(false, "Cannot get filesize.");
			goto FINALLY;
		}
		d.buffer = malloc(size + 1);
		fread(d.buffer, 1, size, file);
	}

FINALLY:
	fclose(file);
}

void BSP::swapBsp()
{
	D(d);
	d.header = (BSPHeader*)d.buffer;
	SwapBlock((int *)d.header, sizeof(*d.header));

	if (d.header->ident != BSP_IDENT) {
		LOG_ASSERT_MSG(false, "Invalid IBSP file");
	}
	if (d.header->version != BSP_VERSION) {
		LOG_ASSERT_MSG(false, "Bad version of bsp.");
	}

	const int extrasize = 1;

	int length = (d.header->lumps[LUMP_SHADERS].filelen) / sizeof(BSPShader);
	d.shaders.resize(length + extrasize);
	d.numShaders = CopyLump(d.header, LUMP_SHADERS, &d.shaders[0], sizeof(BSPShader));

	length = (d.header->lumps[LUMP_MODELS].filelen) / sizeof(BSPModel);
	d.models.resize(length + extrasize);
	d.nummodels = CopyLump(d.header, LUMP_MODELS, &d.models[0], sizeof(BSPModel));

	length = (d.header->lumps[LUMP_PLANES].filelen) / sizeof(BSPPlane);
	d.planes.resize(length + extrasize);
	d.numplanes = CopyLump(d.header, LUMP_PLANES, &d.planes[0], sizeof(BSPPlane));

	length = (d.header->lumps[LUMP_LEAFS].filelen) / sizeof(BSPLeaf);
	d.leafs.resize(length + extrasize);
	d.numleafs = CopyLump(d.header, LUMP_LEAFS, &d.leafs[0], sizeof(BSPLeaf));

	length = (d.header->lumps[LUMP_NODES].filelen) / sizeof(BSPNode);
	d.nodes.resize(length + extrasize);
	d.numnodes = CopyLump(d.header, LUMP_NODES, &d.nodes[0], sizeof(BSPNode));

	length = (d.header->lumps[LUMP_LEAFSURFACES].filelen) / sizeof(d.leafsurfaces[0]);
	d.leafsurfaces.resize(length + extrasize);
	d.numleafsurfaces = CopyLump(d.header, LUMP_LEAFSURFACES, &d.leafsurfaces[0], sizeof(d.leafsurfaces[0]));

	length = (d.header->lumps[LUMP_LEAFBRUSHES].filelen) / sizeof(d.leafbrushes[0]);
	d.leafbrushes.resize(length + extrasize);
	d.numleafbrushes = CopyLump(d.header, LUMP_LEAFBRUSHES, &d.leafbrushes[0], sizeof(d.leafbrushes[0]));

	length = (d.header->lumps[LUMP_BRUSHES].filelen) / sizeof(BSPBrush);
	d.brushes.resize(length + extrasize);
	d.numbrushes = CopyLump(d.header, LUMP_BRUSHES, &d.brushes[0], sizeof(BSPBrush));

	length = (d.header->lumps[LUMP_BRUSHSIDES].filelen) / sizeof(BSPBrushSide);
	d.brushsides.resize(length + extrasize);
	d.numbrushsides = CopyLump(d.header, LUMP_BRUSHSIDES, &d.brushsides[0], sizeof(BSPBrushSide));

	length = (d.header->lumps[LUMP_SURFACES].filelen) / sizeof(BSPSurface);
	d.drawSurfaces.resize(length + extrasize);
	d.numDrawSurfaces = CopyLump(d.header, LUMP_SURFACES, &d.drawSurfaces[0], sizeof(BSPSurface));

	length = (d.header->lumps[LUMP_DRAWINDEXES].filelen) / sizeof(d.drawIndexes[0]);
	d.drawIndexes.resize(length + extrasize);
	d.numDrawIndexes = CopyLump(d.header, LUMP_DRAWINDEXES, &d.drawIndexes[0], sizeof(d.drawIndexes[0]));

	length = (d.header->lumps[LUMP_VISIBILITY].filelen) / 1;
	d.visBytes.resize(length + extrasize);
	d.numVisBytes = CopyLump(d.header, LUMP_VISIBILITY, &d.visBytes[0], 1);

	length = (d.header->lumps[LUMP_LIGHTMAPS].filelen) / 1;
	d.lightBytes.resize(length + extrasize);
	d.numLightBytes = CopyLump(d.header, LUMP_LIGHTMAPS, &d.lightBytes[0], 1);

	length = (d.header->lumps[LUMP_ENTITIES].filelen) / 1;
	d.dentdata.resize(length + extrasize);
	d.dentdatasize = CopyLump(d.header, LUMP_ENTITIES, &d.dentdata[0], 1);

	length = (d.header->lumps[LUMP_LIGHTGRID].filelen) / 1;
	d.gridData.resize(length + extrasize);
	d.numGridPoints = CopyLump(d.header, LUMP_LIGHTGRID, &d.gridData[0], 8);

	// swap everything
	SwapBSPFile(d);
}

void BSP::parseEntities()
{
	D(d);
	d.numentities = 0;
	parseFromMemory(d.dentdata.data(), d.dentdatasize);

	while (parseEntity()) {
	}
}

void BSP::parseFromMemory(char *buffer, int size)
{
	D(d);
	d.script = d.scriptstack;
	d.script++;
	if (d.script == &d.scriptstack[MAX_INCLUDES])
		LOG_ASSERT_MSG(false, "script file exceeded MAX_INCLUDES");
	strcpy(d.script->filename, "memory buffer");

	d.script->buffer = buffer;
	d.script->line = 1;
	d.script->script_p = d.script->buffer;
	d.script->end_p = d.script->buffer + size;

	d.endofscript = false;
	d.tokenready = false;
}

bool BSP::parseEntity()
{
	D(d);

	BSPKeyValuePair *e;
	BSPEntity	*mapent;

	if (!getToken(true)) {
		return false;
	}

	if (strcmp(d.token, "{")) {
		Log::getInstance().warning("parseEntity: { not found");
	}

	BSPEntity bla;
	bla.brushes = 0;
	bla.epairs = 0;
	bla.firstDrawSurf = 0;
	bla.origin[0] = 0.f;
	bla.origin[1] = 0.f;
	bla.origin[2] = 0.f;
	bla.patches = 0;

	d.entities.push_back(bla);
	mapent = &d.entities[d.entities.size() - 1];
	d.numentities++;

	do {
		if (!getToken(true)) {
			Log::getInstance().warning("parseEntity: EOF without closing brace");
		}
		if (!strcmp(d.token, "}")) {
			break;
		}
		e = (struct BSPPair*)parseEpair();
		e->next = mapent->epairs;
		mapent->epairs = e;
	} while (1);

	return true;
}

bool BSP::getToken(bool crossline)
{
	D(d);
	char *token_p;

	if (d.tokenready)                         // is a token allready waiting?
	{
		d.tokenready = false;
		return true;
	}

	if (d.script->script_p >= d.script->end_p)
		return endOfScript(crossline);

	//
	// skip space
	//
skipspace:
	while (*d.script->script_p <= 32)
	{
		if (d.script->script_p >= d.script->end_p)
			return endOfScript(crossline);
		if (*d.script->script_p++ == '\n')
		{
			if (!crossline)
				LOG_ASSERT_MSG(false, "Line %i is incomplete\n", scriptline);
			d.scriptline = d.script->line++;
		}
	}

	if (d.script->script_p >= d.script->end_p)
		return endOfScript(crossline);

	// ; # // comments
	if (*d.script->script_p == ';' || *d.script->script_p == '#'
		|| (d.script->script_p[0] == '/' && d.script->script_p[1] == '/'))
	{
		if (!crossline)
			LOG_ASSERT_MSG(false, "Line %i is incomplete\n", scriptline);
		while (*d.script->script_p++ != '\n')
			if (d.script->script_p >= d.script->end_p)
				return endOfScript(crossline);
		d.scriptline = d.script->line++;
		goto skipspace;
	}

	// /* */ comments
	if (d.script->script_p[0] == '/' && d.script->script_p[1] == '*')
	{
		if (!crossline)
			LOG_ASSERT_MSG(false, "Line %i is incomplete\n", scriptline);
		d.script->script_p += 2;
		while (d.script->script_p[0] != '*' && d.script->script_p[1] != '/')
		{
			if (*d.script->script_p == '\n') {
				d.scriptline = d.script->line++;
			}
			d.script->script_p++;
			if (d.script->script_p >= d.script->end_p)
				return endOfScript(crossline);
		}
		d.script->script_p += 2;
		goto skipspace;
	}

	//
	// copy token
	//
	token_p = d.token;

	if (*d.script->script_p == '"')
	{
		// quoted token
		d.script->script_p++;
		while (*d.script->script_p != '"')
		{
			*token_p++ = *d.script->script_p++;
			if (d.script->script_p == d.script->end_p)
				break;
			if (token_p == &d.token[MAXTOKEN])
				LOG_ASSERT_MSG(false, "Token too large.");
		}
		d.script->script_p++;
	}
	else	// regular token
		while (*d.script->script_p > 32 && *d.script->script_p != ';')
		{
			*token_p++ = *d.script->script_p++;
			if (d.script->script_p == d.script->end_p)
				break;
			if (token_p == &d.token[MAXTOKEN])
				LOG_ASSERT_MSG(false, "Token too large on line %i\n", scriptline);
		}

	*token_p = 0;

	if (!strcmp(d.token, "$include"))
	{
		getToken(false);
		addScriptToStack(d.token);
		return getToken(crossline);
	}

	return true;
}

BSPKeyValuePair* BSP::parseEpair(void)
{
	D(d);
	BSPKeyValuePair	*e;

	e = (struct BSPPair*) malloc(sizeof(BSPKeyValuePair));
	memset(e, 0, sizeof(BSPKeyValuePair));

	if (strlen(d.token) >= MAX_KEY - 1) {
		//printf ("ParseEpar: token too long");
	}
	e->key = copystring(d.token);
	getToken(false);
	if (strlen(d.token) >= MAX_VALUE - 1) {

		//printf ("ParseEpar: token too long");
	}
	e->value = copystring(d.token);

	// strip trailing spaces that sometimes get accidentally
	// added in the editor
	stripTrailing(e->key);
	stripTrailing(e->value);

	return e;
}

void BSP::addScriptToStack(const char *filename)
{
	D(d);
	GMint size;

	d.script++;
	if (d.script == &d.scriptstack[MAX_INCLUDES])
		LOG_ASSERT_MSG(false, "script file exceeded MAX_INCLUDES");
	strcpy(d.script->filename, expandPath(filename).c_str());

	size = LoadFile(d.script->filename, (void **)&d.script->buffer);

	printf("entering %s\n", d.script->filename);

	d.script->line = 1;

	d.script->script_p = d.script->buffer;
	d.script->end_p = d.script->buffer + size;
}


bool BSP::endOfScript(bool crossline)
{
	D(d);
	if (!crossline)
		LOG_ASSERT_MSG(false, "Line %i is incomplete\n", scriptline);

	if (!strcmp(d.script->filename, "memory buffer"))
	{
		d.endofscript = true;
		return false;
	}

	free(d.script->buffer);
	if (d.script == d.scriptstack + 1)
	{
		d.endofscript = true;
		return false;
	}
	d.script--;
	d.scriptline = d.script->line;
	printf("returning to %s\n", d.script->filename);
	return getToken(crossline);
}

const char* BSP::valueForKey(const BSPEntity *ent, const char *key)
{
	BSPPair	*ep;

	for (ep = ent->epairs; ep; ep = ep->next) {
		if (!strcmp(ep->key, key)) {
			return ep->value;
		}
	}
	return "";
}

bool BSP::vectorForKey(const BSPEntity *ent, const char *key, BSPVector3 vec)
{
	const char	*k;
	k = valueForKey(ent, key);
	if (strcmp(k, ""))
	{
		sscanf(k, "%f %f %f", &vec[0], &vec[1], &vec[2]);
		return true;
	}
	return false;
}