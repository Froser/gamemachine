#include "stdafx.h"
#include "bsp_render.h"

BSPRenderData& BSPRender::renderData()
{
	D(d);
	return d;
}


//Tesselate a biquadratic patch
bool BSP_Render_BiquadraticPatch::tesselate(int newTesselation)
{
	tesselation = newTesselation;

	GMfloat px, py;
	BSP_Render_Vertex temp[3];
	vertices = new BSP_Render_Vertex[(tesselation + 1)*(tesselation + 1)];

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

void BSPRender::generateRenderData(BSPData* bsp)
{
	D(d);
	d.bsp = bsp;
	
	generateVertices();
	generateFaces();
	generateShaders();
	generateLightmaps();
	generateLeafs();
	generatePlanes();
	generateVisibilityData();
}

void BSPRender::generateVertices()
{
	D(d);
	// create vertices for drawing
	d.vertices.resize(d.bsp->numDrawVertices);
	for (GMint i = 0; i < d.bsp->numDrawVertices; i++)
	{
		d.vertices[i].position[0] = d.bsp->vertices[i].xyz[0];
		d.vertices[i].position[1] = d.bsp->vertices[i].xyz[1];
		d.vertices[i].position[2] = d.bsp->vertices[i].xyz[2];
		d.vertices[i].position[3] = 1.0f;

		//scale down
		// d.vertices[i].position /= SCALING_DOWN;
		d.vertices[i].decalS = d.bsp->vertices[i].st[0];
		d.vertices[i].decalT = d.bsp->vertices[i].st[1];

		//Transfer lightmap coordinates
		d.vertices[i].lightmapS = d.bsp->vertices[i].lightmap[0];
		d.vertices[i].lightmapT = d.bsp->vertices[i].lightmap[1];

		for (GMuint j = 0; j < 3; j++)
		{
			if (d.vertices[i].position[j] < d.boundMin[j])
				d.boundMin[j] = d.vertices[i].position[j];
			if (d.vertices[i].position[j] > d.boundMax[j])
				d.boundMax[j] = d.vertices[i].position[j];
		}
	}
}

void BSPRender::generateFaces()
{
	D(d);
	// create faces for drawing
	d.faceDirectory.resize(d.bsp->numDrawSurfaces);
	d.facesToDraw.init(d.bsp->numDrawSurfaces);

	for (GMint i = 0; i < d.bsp->numDrawSurfaces; i++)
	{
		if (d.bsp->drawSurfaces[i].surfaceType == MST_PLANAR)
			++d.numPolygonFaces;
		else if (d.bsp->drawSurfaces[i].surfaceType == MST_PATCH)
			++d.numPatches;
		else if (d.bsp->drawSurfaces[i].surfaceType == MST_TRIANGLE_SOUP)
			++d.numMeshFaces;
	}

	d.polygonFaces.resize(d.numPolygonFaces);
	d.meshFaces.resize(d.numMeshFaces);
	d.patches.resize(d.numPatches);
	GMint currentFace = 0;
	GMint currentMeshFace = 0;
	GMint currentPatch = 0;

	for (int i = 0; i < d.bsp->numDrawSurfaces; ++i)
	{
		if (d.bsp->drawSurfaces[i].surfaceType == MST_PLANAR)		//skip this loadFace if it is not a polygon face
		{
			d.polygonFaces[currentFace].firstVertex = d.bsp->drawSurfaces[i].firstVert;
			d.polygonFaces[currentFace].numVertices = d.bsp->drawSurfaces[i].numVerts;
			d.polygonFaces[currentFace].textureIndex = d.bsp->drawSurfaces[i].shaderNum;
			d.polygonFaces[currentFace].lightmapIndex = d.bsp->drawSurfaces[i].lightmapNum;
			d.polygonFaces[currentFace].firstIndex = d.bsp->drawSurfaces[i].firstIndex;
			d.polygonFaces[currentFace].numIndices = d.bsp->drawSurfaces[i].numIndexes;

			//fill in this entry on the face directory
			d.faceDirectory[i].faceType = MST_PLANAR;
			d.faceDirectory[i].typeFaceNumber = currentFace;

			++currentFace;
		}

		if (d.bsp->drawSurfaces[i].surfaceType == MST_TRIANGLE_SOUP)		//skip this loadFace if it is not a mesh face
		{
			d.meshFaces[currentMeshFace].firstVertex = d.bsp->drawSurfaces[i].firstVert;
			d.meshFaces[currentMeshFace].numVertices = d.bsp->drawSurfaces[i].numVerts;
			d.meshFaces[currentMeshFace].textureIndex = d.bsp->drawSurfaces[i].shaderNum;
			d.meshFaces[currentMeshFace].lightmapIndex = d.bsp->drawSurfaces[i].lightmapNum;
			d.meshFaces[currentMeshFace].firstIndex = d.bsp->drawSurfaces[i].firstIndex;
			d.meshFaces[currentMeshFace].numIndices = d.bsp->drawSurfaces[i].numIndexes;

			//fill in this entry on the face directory
			d.faceDirectory[i].faceType = MST_TRIANGLE_SOUP;
			d.faceDirectory[i].typeFaceNumber = currentMeshFace;

			++currentMeshFace;
		}

		if (d.bsp->drawSurfaces[i].surfaceType == MST_PATCH)		//skip this loadFace if it is not a patch
		{
			d.patches[currentPatch].textureIndex = d.bsp->drawSurfaces[i].shaderNum;
			d.patches[currentPatch].lightmapIndex = d.bsp->drawSurfaces[i].lightmapNum;
			d.patches[currentPatch].width = d.bsp->drawSurfaces[i].patchWidth;
			d.patches[currentPatch].height = d.bsp->drawSurfaces[i].patchHeight;

			//fill in this entry on the face directory
			d.faceDirectory[i].faceType = MST_PATCH;
			d.faceDirectory[i].typeFaceNumber = currentPatch;

			//Create space to hold quadratic patches
			// 一个patch有3x3个顶点组成
			int numPatchesWide = (d.patches[currentPatch].width - 1) / 2;
			int numPatchesHigh = (d.patches[currentPatch].height - 1) / 2;

			d.patches[currentPatch].numQuadraticPatches = numPatchesWide*numPatchesHigh;
			d.patches[currentPatch].quadraticPatches = new BSP_Render_BiquadraticPatch
				[d.patches[currentPatch].numQuadraticPatches];
			if (!d.patches[currentPatch].quadraticPatches)
			{
				gm_error("Unable to allocate memory for %d quadratic patches",
					d.patches[currentPatch].numQuadraticPatches);
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
							d.patches[currentPatch].quadraticPatches[y*numPatchesWide + x].
								controlPoints[row * 3 + point] = d.vertices[d.bsp->drawSurfaces[i].firstVert +
								(y * 2 * d.patches[currentPatch].width + x * 2) +
								row*d.patches[currentPatch].width + point];
						}
					}

					//tesselate the patch

					//TODO  curveTesselation
					GMint curveTesselation = 8;
					d.patches[currentPatch].quadraticPatches[y*numPatchesWide + x].tesselate(curveTesselation);
				}
			}

			++currentPatch;
		}
	}
}

void BSPRender::generateShaders()
{
	D(d);
	// Nothing to do
}

void BSPRender::generateLightmaps()
{
	D(d);
	// Nothing to do
}

void BSPRender::generateLeafs()
{
	D(d);
	//leafs
	d.leafs.resize(d.bsp->numleafs);
	for (GMint i = 0; i < d.bsp->numleafs; ++i)
	{
		d.leafs[i].cluster = d.bsp->leafs[i].cluster;
		d.leafs[i].firstLeafFace = d.bsp->leafs[i].firstLeafSurface;
		d.leafs[i].numFaces = d.bsp->leafs[i].numLeafSurfaces;

		//Create the bounding box
		d.leafs[i].boundingBoxVertices[0] = vmath::vec3(d.bsp->leafs[i].mins[0], d.bsp->leafs[i].mins[2], -d.bsp->leafs[i].mins[1]);
		d.leafs[i].boundingBoxVertices[1] = vmath::vec3(d.bsp->leafs[i].mins[0], d.bsp->leafs[i].mins[2], -d.bsp->leafs[i].maxs[1]);
		d.leafs[i].boundingBoxVertices[2] = vmath::vec3(d.bsp->leafs[i].mins[0], d.bsp->leafs[i].maxs[2], -d.bsp->leafs[i].mins[1]);
		d.leafs[i].boundingBoxVertices[3] = vmath::vec3(d.bsp->leafs[i].mins[0], d.bsp->leafs[i].maxs[2], -d.bsp->leafs[i].maxs[1]);
		d.leafs[i].boundingBoxVertices[4] = vmath::vec3(d.bsp->leafs[i].maxs[0], d.bsp->leafs[i].mins[2], -d.bsp->leafs[i].mins[1]);
		d.leafs[i].boundingBoxVertices[5] = vmath::vec3(d.bsp->leafs[i].maxs[0], d.bsp->leafs[i].mins[2], -d.bsp->leafs[i].maxs[1]);
		d.leafs[i].boundingBoxVertices[6] = vmath::vec3(d.bsp->leafs[i].maxs[0], d.bsp->leafs[i].maxs[2], -d.bsp->leafs[i].mins[1]);
		d.leafs[i].boundingBoxVertices[7] = vmath::vec3(d.bsp->leafs[i].maxs[0], d.bsp->leafs[i].maxs[2], -d.bsp->leafs[i].maxs[1]);

		// for (int j = 0; j < 8; ++j)
		// 	d.leafs[i].boundingBoxVertices[j] /= SCALING_DOWN;
	}
}

void BSPRender::generatePlanes()
{
	D(d);
	// Nothing to do
}

void BSPRender::generateVisibilityData()
{
	D(d);
	// visBytes头两个int表示clusters，后面的字节表示bitsets
	size_t sz = sizeof(int) * 2;
	memcpy(&d.visibilityData, d.bsp->visBytes.data(), sz);
	int bitsetSize = d.visibilityData.numClusters * d.visibilityData.bytesPerCluster;
	d.visibilityData.bitset = new GMbyte[bitsetSize];
	memcpy(d.visibilityData.bitset, d.bsp->visBytes.data() + sz, bitsetSize);
}
