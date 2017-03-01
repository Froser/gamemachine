#include "stdafx.h"
#include "bsp.h"
#include <stdio.h>
#include "utilities\assert.h"
#include "utilities\path.h"
#include "bsp_interior.inl"

//Tesselate a biquadratic patch
bool BSP_Drawing_BiquadraticPatch::tesselate(int newTesselation)
{
	tesselation = newTesselation;

	GMfloat px, py;
	BSP_Drawing_Vertex temp[3];
	vertices = new BSP_Drawing_Vertex[(tesselation + 1)*(tesselation + 1)];

	for (int v = 0; v <= tesselation; ++v)
	{
		px = (GMfloat)v / tesselation;

		vertices[v] = controlPoints[0] * ((1.0f - px)*(1.0f - px)) +
			controlPoints[3] * ((1.0f - px)*px * 2) +
			controlPoints[6] * (px*px);
	}

	for (int u = 1; u <= tesselation; ++u)
	{
		py = (GMfloat)u / tesselation;

		temp[0] = controlPoints[0] * ((1.0f - py)*(1.0f - py)) +
			controlPoints[1] * ((1.0f - py)*py * 2) +
			controlPoints[2] * (py*py);

		temp[1] = controlPoints[3] * ((1.0f - py)*(1.0f - py)) +
			controlPoints[4] * ((1.0f - py)*py * 2) +
			controlPoints[5] * (py*py);

		temp[2] = controlPoints[6] * ((1.0f - py)*(1.0f - py)) +
			controlPoints[7] * ((1.0f - py)*py * 2) +
			controlPoints[8] * (py*py);

		for (int v = 0; v <= tesselation; ++v)
		{
			px = (GMfloat)v / tesselation;

			vertices[u*(tesselation + 1) + v] = temp[0] * ((1.0f - px)*(1.0f - px)) +
				temp[1] * ((1.0f - px)*px * 2) +
				temp[2] * (px*px);
		}
	}

	//Create indices
	indices = new GLuint[tesselation*(tesselation + 1) * 2];
	if (!indices)
	{
		gm_error("Unable to allocate memory for surface indices");
		return false;
	}

	for (int row = 0; row < tesselation; ++row)
	{
		for (int point = 0; point <= tesselation; ++point)
		{
			//calculate indices
			//reverse them to reverse winding
			indices[(row*(tesselation + 1) + point) * 2 + 1] = row*(tesselation + 1) + point;
			indices[(row*(tesselation + 1) + point) * 2] = (row + 1)*(tesselation + 1) + point;
		}
	}


	//Fill in the arrays for multi_draw_arrays
	trianglesPerRow = new int[tesselation];
	rowIndexPointers = new unsigned int *[tesselation];
	if (!trianglesPerRow || !rowIndexPointers)
	{
		gm_error("Unable to allocate memory for indices for multi_draw_arrays");
		return false;
	}

	for (int row = 0; row < tesselation; ++row)
	{
		trianglesPerRow[row] = 2 * (tesselation + 1);
		rowIndexPointers[row] = &indices[row * 2 * (tesselation + 1)];
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
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
	parseEntities();

	// 将原始数据组装起来
	generateRenderData();
}

BSPData& BSP::bspData()
{
	D(d);
	return d;
}

void BSP::readFile()
{
	D(d);
	FILE* file = nullptr;
	file = fopen(d.filename.c_str(), "rb");
	if (file)
	{
		GMuint size;
		if (fseek(file, 0, SEEK_END) || (size = ftell(file)) == EOF || fseek(file, 0, SEEK_SET))
		{
			gm_error("Cannot get filesize.");
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
		gm_error("Invalid IBSP file");
	}
	if (d.header->version != BSP_VERSION) {
		gm_error("Bad version of bsp.");
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

	length = (d.header->lumps[LUMP_LEAFSURFACES].filelen) / sizeof(int);
	d.leafsurfaces.resize(length + extrasize);
	d.numleafsurfaces = CopyLump(d.header, LUMP_LEAFSURFACES, &d.leafsurfaces[0], sizeof(int));

	length = (d.header->lumps[LUMP_LEAFBRUSHES].filelen) / sizeof(int);
	d.leafbrushes.resize(length + extrasize);
	d.numleafbrushes = CopyLump(d.header, LUMP_LEAFBRUSHES, &d.leafbrushes[0], sizeof(int));

	length = (d.header->lumps[LUMP_BRUSHES].filelen) / sizeof(BSPBrush);
	d.brushes.resize(length + extrasize);
	d.numbrushes = CopyLump(d.header, LUMP_BRUSHES, &d.brushes[0], sizeof(BSPBrush));

	length = (d.header->lumps[LUMP_BRUSHSIDES].filelen) / sizeof(BSPBrushSide);
	d.brushsides.resize(length + extrasize);
	d.numbrushsides = CopyLump(d.header, LUMP_BRUSHSIDES, &d.brushsides[0], sizeof(BSPBrushSide));

	length = (d.header->lumps[LUMP_DRAWVERTS].filelen) / sizeof(BSPDrawVertices);
	d.vertices.resize(length + extrasize);
	d.numDrawVertices = CopyLump(d.header, LUMP_DRAWVERTS, &d.vertices[0], sizeof(BSPDrawVertices));

	length = (d.header->lumps[LUMP_SURFACES].filelen) / sizeof(BSPSurface);
	d.drawSurfaces.resize(length + extrasize);
	d.numDrawSurfaces = CopyLump(d.header, LUMP_SURFACES, &d.drawSurfaces[0], sizeof(BSPSurface));

	length = (d.header->lumps[LUMP_FOGS].filelen) / sizeof(BSPFog);
	d.fogs.resize(length + extrasize);
	d.numFogs = CopyLump(d.header, LUMP_FOGS, &d.fogs[0], sizeof(BSPFog));

	length = (d.header->lumps[LUMP_DRAWINDEXES].filelen) / sizeof(int);
	d.drawIndexes.resize(length + extrasize);
	d.numDrawIndexes = CopyLump(d.header, LUMP_DRAWINDEXES, &d.drawIndexes[0], sizeof(int));

	length = (d.header->lumps[LUMP_VISIBILITY].filelen) / 1;
	d.visBytes.resize(length + extrasize);
	d.numVisBytes = CopyLump(d.header, LUMP_VISIBILITY, &d.visBytes[0], 1);

	length = (d.header->lumps[LUMP_LIGHTMAPS].filelen) / 1;
	d.lightBytes.resize(length + extrasize);
	d.numLightBytes = CopyLump(d.header, LUMP_LIGHTMAPS, &d.lightBytes[0], 1);

	length = (d.header->lumps[LUMP_ENTITIES].filelen) / 1;
	d.entdata.resize(length + extrasize);
	d.entdatasize = CopyLump(d.header, LUMP_ENTITIES, &d.entdata[0], 1);

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
	parseFromMemory(d.entdata.data(), d.entdatasize);

	while (parseEntity()) {
	}
}

void BSP::parseFromMemory(char *buffer, int size)
{
	D(d);
	d.script = d.scriptstack;
	d.script++;
	if (d.script == &d.scriptstack[MAX_INCLUDES])
		gm_error("script file exceeded MAX_INCLUDES");
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
		gm_warning("parseEntity: { not found");
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
			gm_warning("parseEntity: EOF without closing brace");
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
				gm_error(false, "Line %i is incomplete\n", d.scriptline);
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
			gm_error("Line %i is incomplete\n", d.scriptline);
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
		{
			gm_error("Line %i is incomplete\n", d.scriptline);
			ASSERT(false);
		}
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
				gm_error("Token too large.");
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
				gm_error("Token too large on line %i\n", d.scriptline);
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
		gm_error("script file exceeded MAX_INCLUDES");
	strcpy(d.script->filename, expandPath(filename).c_str());

	size = LoadFile(d.script->filename, (void **)&d.script->buffer);

	gm_info("entering %s\n", d.script->filename);

	d.script->line = 1;

	d.script->script_p = d.script->buffer;
	d.script->end_p = d.script->buffer + size;
}


bool BSP::endOfScript(bool crossline)
{
	D(d);
	if (!crossline)
	{
		gm_warning(false, "Line %i is incomplete\n", d.scriptline);
	}

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
	gm_info("returning to %s\n", d.script->filename);
	return getToken(crossline);
}

void BSP::generateRenderData()
{
	D(d);
	generateVertices();
	generateFaces();
	generateShaders();
	generateLightmaps();
	generateBSPData();
}

void BSP::generateVertices()
{
	D(d);
	// create vertices for drawing
	d.drawingVertices.resize(d.numDrawVertices);
	for (GMint i = 0; i < d.numDrawVertices; i++)
	{
		//swap y and z and negate z
		d.drawingVertices[i].position[0] = d.vertices[i].xyz[0];
		d.drawingVertices[i].position[1] = d.vertices[i].xyz[2];
		d.drawingVertices[i].position[2] = -d.vertices[i].xyz[1];
		d.drawingVertices[i].position[3] = 1.0f;

		//scale down
		d.drawingVertices[i].position /= SCALING_DOWN;

		//Transfer texture coordinates (Invert t)
		d.drawingVertices[i].decalS = d.vertices[i].st[0];
		d.drawingVertices[i].decalT = -d.vertices[i].st[1];

		//Transfer lightmap coordinates
		d.drawingVertices[i].lightmapS = d.vertices[i].lightmap[0];
		d.drawingVertices[i].lightmapT = d.vertices[i].lightmap[1];

		for (GMuint j = 0; j < 3; j++)
		{
			if (d.drawingVertices[i].position[j] < d.boundMin[j])
				d.boundMin[j] = d.drawingVertices[i].position[j];
			if (d.drawingVertices[i].position[j] > d.boundMax[j])
				d.boundMax[j] = d.drawingVertices[i].position[j];
		}
	}
}

void BSP::generateFaces()
{
	D(d);
	// create faces for drawing
	d.drawingFaceDirectory.resize(d.numDrawSurfaces);
	d.facesToDraw.init(d.numDrawSurfaces);

	for (GMint i = 0; i < d.numDrawSurfaces; i++)
	{
		if (d.drawSurfaces[i].surfaceType == MST_PLANAR)
			++d.numPolygonFaces;
		else if (d.drawSurfaces[i].surfaceType == MST_PATCH)
			++d.numPatches;
		else if (d.drawSurfaces[i].surfaceType == MST_TRIANGLE_SOUP)
			++d.numMeshFaces;
	}

	d.drawingPolygonFaces.resize(d.numPolygonFaces);
	d.drawingMeshFaces.resize(d.numMeshFaces);
	d.drawingPatches.resize(d.numPatches);
	GMint currentFace = 0;
	GMint currentMeshFace = 0;
	GMint currentPatch = 0;

	for (int i = 0; i < d.numDrawSurfaces; ++i)
	{
		if (d.drawSurfaces[i].surfaceType == MST_PLANAR)		//skip this loadFace if it is not a polygon face
		{
			d.drawingPolygonFaces[currentFace].firstVertexIndex = d.drawSurfaces[i].firstVert;
			d.drawingPolygonFaces[currentFace].numVertices = d.drawSurfaces[i].numVerts;
			d.drawingPolygonFaces[currentFace].textureIndex = d.drawSurfaces[i].shaderNum;
			d.drawingPolygonFaces[currentFace].lightmapIndex = d.drawSurfaces[i].lightmapNum;

			//fill in this entry on the face directory
			d.drawingFaceDirectory[i].faceType = MST_PLANAR;
			d.drawingFaceDirectory[i].typeFaceNumber = currentFace;

			++currentFace;
		}

		if (d.drawSurfaces[i].surfaceType == MST_TRIANGLE_SOUP)		//skip this loadFace if it is not a mesh face
		{
			d.drawingMeshFaces[currentMeshFace].firstVertexIndex = d.drawSurfaces[i].firstVert;
			d.drawingMeshFaces[currentMeshFace].numVertices = d.drawSurfaces[i].numVerts;
			d.drawingMeshFaces[currentMeshFace].textureIndex = d.drawSurfaces[i].shaderNum;
			d.drawingMeshFaces[currentMeshFace].lightmapIndex = d.drawSurfaces[i].lightmapNum;
			d.drawingMeshFaces[currentMeshFace].firstMeshIndex = d.drawSurfaces[i].firstIndex;
			d.drawingMeshFaces[currentMeshFace].numMeshIndices = d.drawSurfaces[i].numIndexes;

			//fill in this entry on the face directory
			d.drawingFaceDirectory[i].faceType = MST_TRIANGLE_SOUP;
			d.drawingFaceDirectory[i].typeFaceNumber = currentMeshFace;

			++currentMeshFace;
		}

		if (d.drawSurfaces[i].surfaceType == MST_PATCH)		//skip this loadFace if it is not a patch
		{
			d.drawingPatches[currentPatch].textureIndex = d.drawSurfaces[i].shaderNum;
			d.drawingPatches[currentPatch].lightmapIndex = d.drawSurfaces[i].lightmapNum;
			d.drawingPatches[currentPatch].width = d.drawSurfaces[i].patchWidth;
			d.drawingPatches[currentPatch].height = d.drawSurfaces[i].patchHeight;

			//fill in this entry on the face directory
			d.drawingFaceDirectory[i].faceType = MST_PATCH;
			d.drawingFaceDirectory[i].typeFaceNumber = currentPatch;

			//Create space to hold quadratic patches
			int numPatchesWide = (d.drawingPatches[currentPatch].width - 1) / 2;
			int numPatchesHigh = (d.drawingPatches[currentPatch].height - 1) / 2;

			d.drawingPatches[currentPatch].numQuadraticPatches = numPatchesWide*numPatchesHigh;
			d.drawingPatches[currentPatch].quadraticPatches = new BSP_Drawing_BiquadraticPatch
				[d.drawingPatches[currentPatch].numQuadraticPatches];
			if (!d.drawingPatches[currentPatch].quadraticPatches)
			{
				gm_error("Unable to allocate memory for %d quadratic patches",
					d.drawingPatches[currentPatch].numQuadraticPatches);
				return;
			}

			//fill in the quadratic patches
			for (int y = 0; y < numPatchesHigh; ++y)
			{
				for (int x = 0; x < numPatchesWide; ++x)
				{
					for (int row = 0; row < 3; ++row)
					{
						for (int point = 0; point < 3; ++point)
						{
							d.drawingPatches[currentPatch].quadraticPatches[y*numPatchesWide + x].
								controlPoints[row * 3 + point] = d.drawingVertices[d.drawSurfaces[i].firstVert +
								(y * 2 * d.drawingPatches[currentPatch].width + x * 2) +
								row*d.drawingPatches[currentPatch].width + point];
						}
					}

					//tesselate the patch

					//TODO  curveTesselation
					GMint curveTesselation = 8;
					d.drawingPatches[currentPatch].quadraticPatches[y*numPatchesWide + x].tesselate(curveTesselation);
				}
			}

			++currentPatch;
		}
	}
}

void BSP::generateShaders()
{
	D(d);
	// Nothing to do
}

void BSP::generateLightmaps()
{
	D(d);
	// Nothing to do
}

void BSP::generateBSPData()
{
	D(d);
	//leafs
	d.drawingLeafs.resize(d.numleafs);
	for (GMint i = 0; i < d.numleafs; ++i)
	{
		d.drawingLeafs[i].cluster = d.leafs[i].cluster;
		d.drawingLeafs[i].firstLeafFace = d.leafs[i].firstLeafSurface;
		d.drawingLeafs[i].numFaces = d.leafs[i].numLeafSurfaces;

		//Create the bounding box
		d.drawingLeafs[i].boundingBoxVertices[0] = vmath::vec3(d.leafs[i].mins[0], d.leafs[i].mins[2], -d.leafs[i].mins[1]);
		d.drawingLeafs[i].boundingBoxVertices[1] = vmath::vec3(d.leafs[i].mins[0], d.leafs[i].mins[2], -d.leafs[i].maxs[1]);
		d.drawingLeafs[i].boundingBoxVertices[2] = vmath::vec3(d.leafs[i].mins[0], d.leafs[i].maxs[2], -d.leafs[i].mins[1]);
		d.drawingLeafs[i].boundingBoxVertices[3] = vmath::vec3(d.leafs[i].mins[0], d.leafs[i].maxs[2], -d.leafs[i].maxs[1]);
		d.drawingLeafs[i].boundingBoxVertices[4] = vmath::vec3(d.leafs[i].maxs[0], d.leafs[i].mins[2], -d.leafs[i].mins[1]);
		d.drawingLeafs[i].boundingBoxVertices[5] = vmath::vec3(d.leafs[i].maxs[0], d.leafs[i].mins[2], -d.leafs[i].maxs[1]);
		d.drawingLeafs[i].boundingBoxVertices[6] = vmath::vec3(d.leafs[i].maxs[0], d.leafs[i].maxs[2], -d.leafs[i].mins[1]);
		d.drawingLeafs[i].boundingBoxVertices[7] = vmath::vec3(d.leafs[i].maxs[0], d.leafs[i].maxs[2], -d.leafs[i].maxs[1]);

		for (int j = 0; j < 8; ++j)
			d.drawingLeafs[i].boundingBoxVertices[j] /= SCALING_DOWN;
	}

	//planes
	//reverse the intercept on the planes and convert planes to OGL coordinates
	d.drawingPlanes.resize(d.numplanes);
	for (GMint i = 0; i < d.numplanes; ++i)
	{
		d.drawingPlanes[i] = Plane(vmath::vec3(d.planes[i].normal[0], d.planes[i].normal[1], d.planes[i].normal[2]), d.planes[i].dist);
		//swap y and z and negate z
		GMfloat temp = d.drawingPlanes[i].normal[1];
		d.drawingPlanes[i].normal[1] = d.drawingPlanes[i].normal[2];
		d.drawingPlanes[i].normal[2] = -temp;

		d.drawingPlanes[i].intercept = -d.drawingPlanes[i].intercept;
		d.drawingPlanes[i].intercept /= SCALING_DOWN;	//scale down
	}

	// visBytes头两个int表示clusters，后面的字节表示bitsets
	size_t sz = sizeof(int) * 2;
	memcpy(&d.visibilityData, d.visBytes.data(), sz);
	int bitsetSize = d.visibilityData.numClusters * d.visibilityData.bytesPerCluster;
	d.visibilityData.bitset = new GMbyte[bitsetSize];
	// TODO: read bitset
	memcpy(d.visibilityData.bitset, d.visBytes.data() + sz, bitsetSize);
}