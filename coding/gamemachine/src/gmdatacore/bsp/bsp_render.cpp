#include "stdafx.h"
#include "bsp_render.h"
#include "gmdatacore/object.h"

GMBSPRenderData& BSPRender::renderData()
{
	D(d);
	return *d;
}

//Tesselate a biquadratic patch
bool BSP_Render_BiquadraticPatch::tesselate(GMint newTesselation)
{
	tesselation = newTesselation;

	GMfloat px, py;
	BSP_Render_Vertex temp[3];
	vertices = new BSP_Render_Vertex[(tesselation + 1)*(tesselation + 1)];

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
	indices = new GMuint[tesselation*(tesselation + 1) * 2];
	if (!indices)
	{
		gm_error("Unable to allocate memory for surface indices");
		return false;
	}

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
		gm_error("Unable to allocate memory for indices for multi_draw_arrays");
		return false;
	}

	for (GMint row = 0; row < tesselation; ++row)
	{
		trianglesPerRow[row] = 2 * (tesselation + 1);
		rowIndexPointers[row] = &indices[row * 2 * (tesselation + 1)];
	}

	return true;
}

void BSPRender::generateRenderData(BSPData* bsp)
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

void BSPRender::generateVertices()
{
	D(d);
	// create vertices for drawing
	d->vertices.resize(d->bsp->numDrawVertices);
	for (GMint i = 0; i < d->bsp->numDrawVertices; i++)
	{
		d->vertices[i].position[0] = d->bsp->vertices[i].xyz[0];
		d->vertices[i].position[1] = d->bsp->vertices[i].xyz[1];
		d->vertices[i].position[2] = d->bsp->vertices[i].xyz[2];
		d->vertices[i].position[3] = 1.0f;

		//scale down
		// d->vertices[i].position /= SCALING_DOWN;
		d->vertices[i].decalS = d->bsp->vertices[i].st[0];
		d->vertices[i].decalT = d->bsp->vertices[i].st[1];

		//Transfer lightmap coordinates
		d->vertices[i].lightmapS = d->bsp->vertices[i].lightmap[0];
		d->vertices[i].lightmapT = d->bsp->vertices[i].lightmap[1];

		for (GMuint j = 0; j < 3; j++)
		{
			if (d->vertices[i].position[j] < d->boundMin[j])
				d->boundMin[j] = d->vertices[i].position[j];
			if (d->vertices[i].position[j] > d->boundMax[j])
				d->boundMax[j] = d->vertices[i].position[j];
		}
	}
}

void BSPRender::generateFaces()
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
			d->patches[currentPatch].quadraticPatches = new BSP_Render_BiquadraticPatch
				[d->patches[currentPatch].numQuadraticPatches];
			if (!d->patches[currentPatch].quadraticPatches)
			{
				gm_error("Unable to allocate memory for %d quadratic patches",
					d->patches[currentPatch].numQuadraticPatches);
				return;
			}

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

void BSPRender::generateShaders()
{
}

void BSPRender::generateLightmaps()
{
}

void BSPRender::generateLeafs()
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
		d->leafs[i].boundingBoxVertices[0] = linear_math::Vector3(d->bsp->leafs[i].mins[0], d->bsp->leafs[i].mins[2], -d->bsp->leafs[i].mins[1]);
		d->leafs[i].boundingBoxVertices[1] = linear_math::Vector3(d->bsp->leafs[i].mins[0], d->bsp->leafs[i].mins[2], -d->bsp->leafs[i].maxs[1]);
		d->leafs[i].boundingBoxVertices[2] = linear_math::Vector3(d->bsp->leafs[i].mins[0], d->bsp->leafs[i].maxs[2], -d->bsp->leafs[i].mins[1]);
		d->leafs[i].boundingBoxVertices[3] = linear_math::Vector3(d->bsp->leafs[i].mins[0], d->bsp->leafs[i].maxs[2], -d->bsp->leafs[i].maxs[1]);
		d->leafs[i].boundingBoxVertices[4] = linear_math::Vector3(d->bsp->leafs[i].maxs[0], d->bsp->leafs[i].mins[2], -d->bsp->leafs[i].mins[1]);
		d->leafs[i].boundingBoxVertices[5] = linear_math::Vector3(d->bsp->leafs[i].maxs[0], d->bsp->leafs[i].mins[2], -d->bsp->leafs[i].maxs[1]);
		d->leafs[i].boundingBoxVertices[6] = linear_math::Vector3(d->bsp->leafs[i].maxs[0], d->bsp->leafs[i].maxs[2], -d->bsp->leafs[i].mins[1]);
		d->leafs[i].boundingBoxVertices[7] = linear_math::Vector3(d->bsp->leafs[i].maxs[0], d->bsp->leafs[i].maxs[2], -d->bsp->leafs[i].maxs[1]);

		// for (GMint j = 0; j < 8; ++j)
		// 	d->leafs[i].boundingBoxVertices[j] /= SCALING_DOWN;
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
	// visBytes头两个GMint表示clusters，后面的字节表示bitsets
	size_t sz = sizeof(GMint) * 2;
	memcpy(&d->visibilityData, d->bsp->visBytes.data(), sz);
	GMint bitsetSize = d->visibilityData.numClusters * d->visibilityData.bytesPerCluster;
	d->visibilityData.bitset = new GMbyte[bitsetSize];
	memcpy(d->visibilityData.bitset, d->bsp->visBytes.data() + sz, bitsetSize);
}

void BSPRender::createObject(const BSP_Render_Face& face, const Shader& shader, OUT Object** obj)
{
	D(d);

	Object* coreObj = new Object();
	Mesh* child = new Mesh();
	child->setArrangementMode(Mesh::Triangles);
	Component* component = new Component(child);
	component->getShader() = shader;

	ASSERT(face.numIndices % 3 == 0);
	for (GMint i = 0; i < face.numIndices / 3; i++)
	{
		component->beginFace();
		for (GMint j = 0; j < 3; j++)
		{
			GMint idx = d->bsp->drawIndexes[face.firstIndex + i * 3 + j];
			BSP_Render_Vertex& vertex = d->vertices[face.firstVertex + idx];
			
			GMint idx_prev = d->bsp->drawIndexes[face.firstIndex + i * 3 + (j + 1) % 3];
			GMint idx_next = d->bsp->drawIndexes[face.firstIndex + i * 3 + (j + 2) % 3];
			linear_math::Vector3& vertex_prev = d->vertices[face.firstVertex + idx_prev].position,
				&vertex_next = d->vertices[face.firstVertex + idx_next].position;
			linear_math::Vector3 normal = linear_math::cross(vertex.position - vertex_prev, vertex_next - vertex.position);
			normal = linear_math::normalize(normal);

			component->vertex(vertex.position[0], vertex.position[1], vertex.position[2]);
			component->normal(normal[0], normal[1], normal[2]);
			component->uv(vertex.decalS, vertex.decalT);
			component->lightmap(vertex.lightmapS, vertex.lightmapT);
		}
		component->endFace();
	}

	child->appendComponent(component);
	coreObj->append(child);

	*obj = coreObj;
}

void BSPRender::createObject(const BSP_Render_BiquadraticPatch& biqp, const Shader& shader, OUT Object** obj)
{
	Object* coreObj = new Object();
	Mesh* child = new Mesh();
	child->setArrangementMode(Mesh::Triangle_Strip);

	Component* component = new Component(child);
	component->getShader() = shader;

	GMint numVertices = 2 * (biqp.tesselation + 1);
	for (GMint row = 0; row < biqp.tesselation; ++row)
	{
		component->beginFace();
		GMuint* idxStart = &biqp.indices[row * 2 * (biqp.tesselation + 1)];
		linear_math::Vector3 normal;
		for (GMint i = 0; i < numVertices; i++)
		{
			GMint idx = *(idxStart + i);
			BSP_Render_Vertex& vertex = biqp.vertices[idx];

			if (i < numVertices - 2)
			{
				GMint idx_prev = *(idxStart + i + 2);
				GMint idx_next = *(idxStart + i + 1);

				if (i & 1) //奇数点应该调换一下前后向量，最后再改变法线方向
					SWAP(idx_prev, idx_next);

				linear_math::Vector3& vertex_prev = biqp.vertices[idx_prev].position,
					&vertex_next = biqp.vertices[idx_next].position;
				normal = -linear_math::normalize(linear_math::cross(vertex.position - vertex_prev, vertex_next - vertex.position));
			}
			component->vertex(vertex.position[0], vertex.position[1], vertex.position[2]);
			component->normal(normal[0], normal[1], normal[2]);
			component->uv(vertex.decalS, vertex.decalT);
			component->lightmap(vertex.lightmapS, vertex.lightmapT);
		}
		component->endFace();

	}
	child->appendComponent(component);
	coreObj->append(child);
	*obj = coreObj;
}

void BSPRender::createBox(const linear_math::Vector3& extents, const linear_math::Vector3& position, const Shader& shader, OUT Object** obj)
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

	Object* coreObj = new Object();
	Mesh* child = new Mesh();
	child->setArrangementMode(Mesh::Triangle_Strip);

	GMfloat t[24];
	for (GMint i = 0; i < 24; i++)
	{
		t[i] = extents[i % 3] * v[i] + position[i % 3];
	}

	Component* component = new Component(child);
	component->getShader() = shader;

	linear_math::Vector3 normal;
	for (GMint i = 0; i < 12; i++)
	{
		component->beginFace();
		for (GMint j = 0; j < 3; j++) // j表示面的一个顶点
		{
			GMint idx = i * 3 + j; //顶点的开始
			GMint idx_next = i * 3 + (j + 1) % 3;
			GMint idx_prev = i * 3 + (j + 2) % 3;
			linear_math::Vector3 vertex(t[indices[idx] * 3], t[indices[idx] * 3 + 1], t[indices[idx] * 3 + 2]);
			linear_math::Vector3 vertex_prev(t[indices[idx_prev] * 3], t[indices[idx_prev] * 3 + 1], t[indices[idx_prev] * 3 + 2]),
				vertex_next(t[indices[idx_next] * 3], t[indices[idx_next] * 3 + 1], t[indices[idx_next] * 3 + 2]);
			linear_math::Vector3 normal = linear_math::cross(vertex - vertex_prev, vertex_next - vertex);
			normal = linear_math::normalize(normal);

			component->vertex(vertex[0], vertex[1], vertex[2]);
			component->normal(normal[0], normal[1], normal[2]);
			//TODO
			//component->uv(vertex.decalS, vertex.decalT);
			//component->lightmap(1.f, 1.f);
		}
		component->endFace();
	}

	child->appendComponent(component);
	coreObj->append(child);
	*obj = coreObj;
}
