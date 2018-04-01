#include "stdafx.h"
#include "gmbsp_render.h"
#include "gmdata/gmmodel.h"

GMBSPRenderData& GMBSPRender::renderData()
{
	D(d);
	return *d;
}

//Tesselate a biquadratic patch
bool GMBSP_Render_BiquadraticPatch::tesselate(GMint newTesselation)
{
	tesselation = newTesselation;

	GMfloat px, py;
	GMBSP_Render_Vertex temp[3];
	vertices.resize((tesselation + 1)*(tesselation + 1));

	for (GMint v = 0; v <= tesselation; ++v)
	{
		px = (GMfloat)v / tesselation;

		vertices[v] = controlPoints[0] * ((1.0f - px)*(1.0f - px)) +
			controlPoints[3] * ((1.0f - px)*px * 2) +
			controlPoints[6] * (px*px);
	}

	for (GMint u = 1; u <= tesselation; ++u)
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

		for (GMint v = 0; v <= tesselation; ++v)
		{
			px = (GMfloat)v / tesselation;

			vertices[u*(tesselation + 1) + v] = temp[0] * ((1.0f - px)*(1.0f - px)) +
				temp[1] * ((1.0f - px)*px * 2) +
				temp[2] * (px*px);
		}
	}

	//Create indices
	indices.resize(tesselation*(tesselation + 1) * 2);
	for (GMint row = 0; row < tesselation; ++row)
	{
		for (GMint point = 0; point <= tesselation; ++point)
		{
			//calculate indices
			//reverse them to reverse winding
			indices[(row*(tesselation + 1) + point) * 2 + 1] = row*(tesselation + 1) + point;
			indices[(row*(tesselation + 1) + point) * 2] = (row + 1)*(tesselation + 1) + point;
		}
	}


	//Fill in the arrays for multi_draw_arrays
	trianglesPerRow = new GMint[tesselation];
	rowIndexPointers = new GMuint *[tesselation];
	if (!trianglesPerRow || !rowIndexPointers)
	{
		gm_error(L"Unable to allocate memory for indices for multi_draw_arrays");
		return false;
	}

	for (GMint row = 0; row < tesselation; ++row)
	{
		trianglesPerRow[row] = 2 * (tesselation + 1);
		rowIndexPointers[row] = &indices[row * 2 * (tesselation + 1)];
	}

	return true;
}

void GMBSPRender::generateRenderData(BSPData* bsp)
{
	D(d);
	d->bsp = bsp;
	
	generateVertices();
	generateFaces();
	generateShaders();
	generateLightmaps();
	generateLeafs();
	generatePlanes();
	generateVisibilityData();
}

void GMBSPRender::generateVertices()
{
	D(d);
	// create vertices for drawing
	d->vertices.resize(d->bsp->numDrawVertices);
	GMFloat4 f4_position;
	for (GMint i = 0; i < d->bsp->numDrawVertices; i++)
	{
		d->vertices[i].position = d->bsp->vertices[i].xyz;

		d->vertices[i].decalS = d->bsp->vertices[i].st[0];
		d->vertices[i].decalT = d->bsp->vertices[i].st[1];

		//Transfer lightmap coordinates
		d->vertices[i].lightmapS = d->bsp->vertices[i].lightmap[0];
		d->vertices[i].lightmapT = d->bsp->vertices[i].lightmap[1];

		d->vertices[i].position.loadFloat4(f4_position);
		for (GMuint j = 0; j < 3; j++)
		{
			if (f4_position[j] < d->boundMin[j])
				d->boundMin[j] = f4_position[j];
			if (f4_position[j] > d->boundMax[j])
				d->boundMax[j] = f4_position[j];
		}
	}
}

void GMBSPRender::generateFaces()
{
	D(d);
	// create faces for drawing
	d->faceDirectory.resize(d->bsp->numDrawSurfaces);
	d->facesToDraw.init(d->bsp->numDrawSurfaces);
	d->entitiesToDraw.init(d->bsp->numleafs);

	for (GMint i = 0; i < d->bsp->numDrawSurfaces; i++)
	{
		if (d->bsp->drawSurfaces[i].surfaceType == MST_PLANAR)
			++d->numPolygonFaces;
		else if (d->bsp->drawSurfaces[i].surfaceType == MST_PATCH)
			++d->numPatches;
		else if (d->bsp->drawSurfaces[i].surfaceType == MST_TRIANGLE_SOUP)
			++d->numMeshFaces;
	}

	d->polygonFaces.resize(d->numPolygonFaces);
	d->meshFaces.resize(d->numMeshFaces);
	d->patches.resize(d->numPatches);
	GMint currentFace = 0;
	GMint currentMeshFace = 0;
	GMint currentPatch = 0;

	for (GMint i = 0; i < d->bsp->numDrawSurfaces; ++i)
	{
		if (d->bsp->drawSurfaces[i].surfaceType == MST_PLANAR)		//skip this loadFace if it is not a polygon face
		{
			d->polygonFaces[currentFace].firstVertex = d->bsp->drawSurfaces[i].firstVert;
			d->polygonFaces[currentFace].numVertices = d->bsp->drawSurfaces[i].numVerts;
			d->polygonFaces[currentFace].textureIndex = d->bsp->drawSurfaces[i].shaderNum;
			d->polygonFaces[currentFace].lightmapIndex = d->bsp->drawSurfaces[i].lightmapNum;
			d->polygonFaces[currentFace].firstIndex = d->bsp->drawSurfaces[i].firstIndex;
			d->polygonFaces[currentFace].numIndices = d->bsp->drawSurfaces[i].numIndexes;

			//fill in this entry on the face directory
			d->faceDirectory[i].faceType = MST_PLANAR;
			d->faceDirectory[i].typeFaceNumber = currentFace;

			++currentFace;
		}

		if (d->bsp->drawSurfaces[i].surfaceType == MST_TRIANGLE_SOUP)		//skip this loadFace if it is not a mesh face
		{
			d->meshFaces[currentMeshFace].firstVertex = d->bsp->drawSurfaces[i].firstVert;
			d->meshFaces[currentMeshFace].numVertices = d->bsp->drawSurfaces[i].numVerts;
			d->meshFaces[currentMeshFace].textureIndex = d->bsp->drawSurfaces[i].shaderNum;
			d->meshFaces[currentMeshFace].lightmapIndex = d->bsp->drawSurfaces[i].lightmapNum;
			d->meshFaces[currentMeshFace].firstIndex = d->bsp->drawSurfaces[i].firstIndex;
			d->meshFaces[currentMeshFace].numIndices = d->bsp->drawSurfaces[i].numIndexes;

			//fill in this entry on the face directory
			d->faceDirectory[i].faceType = MST_TRIANGLE_SOUP;
			d->faceDirectory[i].typeFaceNumber = currentMeshFace;

			++currentMeshFace;
		}

		if (d->bsp->drawSurfaces[i].surfaceType == MST_PATCH)		//skip this loadFace if it is not a patch
		{
			d->patches[currentPatch].textureIndex = d->bsp->drawSurfaces[i].shaderNum;
			d->patches[currentPatch].lightmapIndex = d->bsp->drawSurfaces[i].lightmapNum;
			d->patches[currentPatch].width = d->bsp->drawSurfaces[i].patchWidth;
			d->patches[currentPatch].height = d->bsp->drawSurfaces[i].patchHeight;

			//fill in this entry on the face directory
			d->faceDirectory[i].faceType = MST_PATCH;
			d->faceDirectory[i].typeFaceNumber = currentPatch;

			//Create space to hold quadratic patches
			// 一个patch有3x3个顶点组成
			GMint numPatchesWide = (d->patches[currentPatch].width - 1) / 2;
			GMint numPatchesHigh = (d->patches[currentPatch].height - 1) / 2;

			d->patches[currentPatch].numQuadraticPatches = numPatchesWide*numPatchesHigh;
			d->patches[currentPatch].quadraticPatches.resize(d->patches[currentPatch].numQuadraticPatches);

			//fill in the quadratic patches
			for (GMint y = 0; y < numPatchesHigh; ++y)
			{
				for (GMint x = 0; x < numPatchesWide; ++x)
				{
					for (GMint row = 0; row < 3; ++row)
					{
						for (GMint point = 0; point < 3; ++point)
						{
							d->patches[currentPatch].quadraticPatches[y*numPatchesWide + x].
								controlPoints[row * 3 + point] = d->vertices[d->bsp->drawSurfaces[i].firstVert +
								(y * 2 * d->patches[currentPatch].width + x * 2) +
								row*d->patches[currentPatch].width + point];
						}
					}

					//tesselate the patch

					//TODO  curveTesselation
					GMint curveTesselation = 8;
					d->patches[currentPatch].quadraticPatches[y*numPatchesWide + x].tesselate(curveTesselation);
				}
			}

			++currentPatch;
		}
	}
}

void GMBSPRender::generateShaders()
{
}

void GMBSPRender::generateLightmaps()
{
}

void GMBSPRender::generateLeafs()
{
	D(d);
	//leafs
	d->leafs.resize(d->bsp->numleafs);
	for (GMint i = 0; i < d->bsp->numleafs; ++i)
	{
		d->leafs[i].cluster = d->bsp->leafs[i].cluster;
		d->leafs[i].firstLeafFace = d->bsp->leafs[i].firstLeafSurface;
		d->leafs[i].numFaces = d->bsp->leafs[i].numLeafSurfaces;

		//Create the bounding box
		d->leafs[i].boundingBoxVertices[0] = GMVec3(d->bsp->leafs[i].mins[0], d->bsp->leafs[i].mins[2], d->bsp->leafs[i].mins[1]);
		d->leafs[i].boundingBoxVertices[1] = GMVec3(d->bsp->leafs[i].mins[0], d->bsp->leafs[i].mins[2], d->bsp->leafs[i].maxs[1]);
		d->leafs[i].boundingBoxVertices[2] = GMVec3(d->bsp->leafs[i].mins[0], d->bsp->leafs[i].maxs[2], d->bsp->leafs[i].mins[1]);
		d->leafs[i].boundingBoxVertices[3] = GMVec3(d->bsp->leafs[i].mins[0], d->bsp->leafs[i].maxs[2], d->bsp->leafs[i].maxs[1]);
		d->leafs[i].boundingBoxVertices[4] = GMVec3(d->bsp->leafs[i].maxs[0], d->bsp->leafs[i].mins[2], d->bsp->leafs[i].mins[1]);
		d->leafs[i].boundingBoxVertices[5] = GMVec3(d->bsp->leafs[i].maxs[0], d->bsp->leafs[i].mins[2], d->bsp->leafs[i].maxs[1]);
		d->leafs[i].boundingBoxVertices[6] = GMVec3(d->bsp->leafs[i].maxs[0], d->bsp->leafs[i].maxs[2], d->bsp->leafs[i].mins[1]);
		d->leafs[i].boundingBoxVertices[7] = GMVec3(d->bsp->leafs[i].maxs[0], d->bsp->leafs[i].maxs[2], d->bsp->leafs[i].maxs[1]);

		// for (GMint j = 0; j < 8; ++j)
		// 	d->leafs[i].boundingBoxVertices[j] /= SCALING_DOWN;
	}
}

void GMBSPRender::generatePlanes()
{
	D(d);
	// Nothing to do
}

void GMBSPRender::generateVisibilityData()
{
	D(d);
	// visBytes头两个GMint表示clusters，后面的字节表示bitsets
	size_t sz = sizeof(GMint) * 2;
	memcpy(&d->visibilityData, d->bsp->visBytes.data(), sz);
	GMint bitsetSize = d->visibilityData.numClusters * d->visibilityData.bytesPerCluster;
	d->visibilityData.bitset = new GMbyte[bitsetSize];
	memcpy(d->visibilityData.bitset, d->bsp->visBytes.data() + sz, bitsetSize);
}

void GMBSPRender::createObject(const GMBSP_Render_Face& face, const GMShader& shader, OUT GMModel** obj)
{
	D(d);
	GMModel* model = new GMModel();
	GMMesh* mesh = model->getMesh();
	mesh->setArrangementMode(GMArrangementMode::Triangles);
	GMComponent* component = new GMComponent(mesh);
	component->setShader(shader);

	GMFloat4 f4_position, f4_normal;
	GM_ASSERT(face.numIndices % 3 == 0);
	for (GMint i = 0; i < face.numIndices / 3; i++)
	{
		component->beginFace();
		for (GMint j = 0; j < 3; j++)
		{
			GMint idx = d->bsp->drawIndexes[face.firstIndex + i * 3 + j];
			GMBSP_Render_Vertex& vertex = d->vertices[face.firstVertex + idx];
			
			GMint idx_prev = d->bsp->drawIndexes[face.firstIndex + i * 3 + (j + 1) % 3];
			GMint idx_next = d->bsp->drawIndexes[face.firstIndex + i * 3 + (j + 2) % 3];
			GMVec3& vertex_prev = d->vertices[face.firstVertex + idx_prev].position,
				&vertex_next = d->vertices[face.firstVertex + idx_next].position;
			GMVec3 normal = Cross(vertex.position - vertex_prev, vertex_next - vertex.position);
			normal = -FastNormalize(normal);

			vertex.position.loadFloat4(f4_position);
			normal.loadFloat4(f4_normal);
			component->vertex(f4_position[0], f4_position[1], f4_position[2]);
			component->normal(f4_normal[0], f4_normal[1], f4_normal[2]);
			component->texcoord(vertex.decalS, vertex.decalT);
			component->lightmap(vertex.lightmapS, vertex.lightmapT);
		}
		component->endFace();
	}
	*obj = model;
}

void GMBSPRender::createObject(const GMBSP_Render_BiquadraticPatch& biqp, const GMShader& shader, OUT GMModel** obj)
{
	GMModel* model = new GMModel();
	GMMesh* mesh = model->getMesh();
	mesh->setArrangementMode(GMArrangementMode::Triangle_Strip);

	GMComponent* component = new GMComponent(mesh);
	component->setShader(shader);

	GMint numVertices = 2 * (biqp.tesselation + 1);

	GMFloat4 f4_position, f4_normal;
	for (GMint row = 0; row < biqp.tesselation; ++row)
	{
		component->beginFace();
		const GMuint* idxStart = &biqp.indices[row * 2 * (biqp.tesselation + 1)];
		GMVec3 normal;
		for (GMint i = 0; i < numVertices; i++)
		{
			GMint idx = *(idxStart + i);
			const GMBSP_Render_Vertex& vertex = biqp.vertices[idx];

			if (i < numVertices - 2)
			{
				GMint idx_prev = *(idxStart + i + 2);
				GMint idx_next = *(idxStart + i + 1);

				if (i & 1) //奇数点应该调换一下前后向量，最后再改变法线方向
					GM_SWAP(idx_prev, idx_next);

				const GMVec3& vertex_prev = biqp.vertices[idx_prev].position,
					&vertex_next = biqp.vertices[idx_next].position;
				normal = -FastNormalize(Cross(vertex.position - vertex_prev, vertex_next - vertex.position));
			}

			vertex.position.loadFloat4(f4_position);
			normal.loadFloat4(f4_normal);
			component->vertex(f4_position[0], f4_position[1], f4_position[2]);
			component->normal(f4_normal[0], f4_normal[1], f4_normal[2]);
			component->texcoord(vertex.decalS, vertex.decalT);
			component->lightmap(vertex.lightmapS, vertex.lightmapT);
		}
		component->endFace();
	}
	*obj = model;
}

void GMBSPRender::createBox(const GMVec3& extents, const GMVec3& position, const GMShader& shader, OUT GMModel** obj)
{
	static GMfloat v[24] = {
		1, -1, 1,
		1, -1, -1,
		-1, -1, 1,
		-1, -1, -1,
		1, 1, 1,
		1, 1, -1,
		-1, 1, 1,
		-1, 1, -1,
	};
	static GMint indices[] = {
		0, 2, 1,
		2, 3, 1,
		4, 5, 6,
		6, 5, 7,
		0, 1, 4,
		1, 5, 4,
		2, 6, 3,
		3, 6, 7,
		0, 4, 2,
		2, 4, 6,
		1, 3, 5,
		3, 7, 5,
	};

	GMModel* model = new GMModel();
	GMMesh* mesh = model->getMesh();
	mesh->setArrangementMode(GMArrangementMode::Triangle_Strip);

	GMFloat4 f4_extents, f4_position;
	extents.loadFloat4(f4_extents);
	position.loadFloat4(f4_position);

	GMfloat t[24];
	for (GMint i = 0; i < 24; i++)
	{
		t[i] = f4_extents[i % 3] * v[i] + f4_position[i % 3];
	}

	GMComponent* component = new GMComponent(mesh);
	component->setShader(shader);

	GMFloat4 f4_vertex, f4_normal;
	for (GMint i = 0; i < 12; i++)
	{
		component->beginFace();
		for (GMint j = 0; j < 3; j++) // j表示面的一个顶点
		{
			GMint idx = i * 3 + j; //顶点的开始
			GMint idx_next = i * 3 + (j + 1) % 3;
			GMint idx_prev = i * 3 + (j + 2) % 3;
			GMVec3 vertex(t[indices[idx] * 3], t[indices[idx] * 3 + 1], t[indices[idx] * 3 + 2]);
			GMVec3 vertex_prev(t[indices[idx_prev] * 3], t[indices[idx_prev] * 3 + 1], t[indices[idx_prev] * 3 + 2]),
				vertex_next(t[indices[idx_next] * 3], t[indices[idx_next] * 3 + 1], t[indices[idx_next] * 3 + 2]);
			GMVec3 normal = Cross(vertex - vertex_prev, vertex_next - vertex);
			normal = -FastNormalize(normal);
			GM_ASSERT(LengthSq(normal) > 1);

			vertex.loadFloat4(f4_vertex);
			normal.loadFloat4(f4_normal);
			component->vertex(f4_vertex[0], f4_vertex[1], f4_vertex[2]);
			component->normal(f4_normal[0], f4_normal[1], f4_normal[2]);
			//TODO
			//component->uv(vertex.decalS, vertex.decalT);
			//component->lightmap(1.f, 1.f);
		}
		component->endFace();
	}
	*obj = model;
}
