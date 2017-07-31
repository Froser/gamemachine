#ifndef __BSP_RENDER_H__
#define __BSP_RENDER_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "foundation/utilities/utilities.h"
#include "foundation/vector.h"
#include <map>
#include "gmbsp.h"
BEGIN_NS

// structs for rendering
GM_ALIGNED_STRUCT(GMBSP_Render_Vertex)
{
	linear_math::Vector3 position;
	GMfloat decalS, decalT;
	GMfloat lightmapS, lightmapT;

	GMBSP_Render_Vertex operator+(const GMBSP_Render_Vertex & rhs) const
	{
		GMBSP_Render_Vertex result;
		result.position = position + rhs.position;
		result.decalS = decalS + rhs.decalS;
		result.decalT = decalT + rhs.decalT;
		result.lightmapS = lightmapS + rhs.lightmapS;
		result.lightmapT = lightmapT + rhs.lightmapT;

		return result;
	}

	GMBSP_Render_Vertex operator*(const float rhs) const
	{
		GMBSP_Render_Vertex result;
		result.position = position*rhs;
		result.decalS = decalS*rhs;
		result.decalT = decalT*rhs;
		result.lightmapS = lightmapS*rhs;
		result.lightmapT = lightmapT*rhs;

		return result;
	}
};

GM_ALIGNED_STRUCT(GMBSP_Render_Face)
{
	GMint firstVertex;
	GMint numVertices;
	GMint textureIndex;
	GMint lightmapIndex;
	GMint firstIndex;
	GMint numIndices;
};

GM_ALIGNED_STRUCT(GMBSP_Render_FaceDirectoryEntry)
{
	GMBSPSurfaceType faceType;
	GMint typeFaceNumber;		//face number in the list of faces of this type
};

//every patch (curved surface) is split into biquadratic (3x3) patches
GM_ALIGNED_STRUCT(GMBSP_Render_BiquadraticPatch)
{
	GMBSP_Render_BiquadraticPatch()
	{
	}

	~GMBSP_Render_BiquadraticPatch()
	{
		if (trianglesPerRow)
			delete[] trianglesPerRow;
		trianglesPerRow = nullptr;

		if (rowIndexPointers)
			delete[] rowIndexPointers;
		rowIndexPointers = nullptr;
	}

	bool tesselate(int newTesselation);

	GMBSP_Render_Vertex controlPoints[9];
	GMint tesselation = 0;
	AlignedVector<GMBSP_Render_Vertex> vertices;
	AlignedVector<GMuint> indices;
	//arrays for multi_draw_arrays
	GMint* trianglesPerRow = nullptr;
	GMuint** rowIndexPointers = nullptr;
};

//curved surface
GM_ALIGNED_STRUCT(GMBSP_Render_Patch)
{
	GMint textureIndex;
	GMint lightmapIndex;
	GMint width, height;
	GMint numQuadraticPatches;
	AlignedVector<GMBSP_Render_BiquadraticPatch> quadraticPatches;
};

enum
{
	BOUNDING = 99999,
};

GM_ALIGNED_STRUCT(GMBSP_Render_Leaf)
{
	linear_math::Vector3 boundingBoxVertices[8];
	GMint cluster;	//cluster index for visdata
	GMint firstLeafFace;	//first index in leafFaces array
	GMint numFaces;
};

struct GMBSP_Render_VisibilityData
{
	GMBSP_Render_VisibilityData() 
		: bitset(nullptr)
	{
	}
	~GMBSP_Render_VisibilityData()
	{
		if (bitset)
			delete[] bitset;
		bitset = nullptr;
	}
	GMint numClusters;
	GMint bytesPerCluster;
	GMbyte * bitset;
};

class GMEntityObject;
GM_PRIVATE_OBJECT(GMBSPRender)
{
	AlignedVector<GMBSP_Render_Vertex> vertices;
	AlignedVector<GMBSP_Render_FaceDirectoryEntry> faceDirectory;
	AlignedVector<GMBSP_Render_Face> polygonFaces;
	AlignedVector<GMBSP_Render_Face> meshFaces;
	AlignedVector<GMBSP_Render_Patch> patches;
	AlignedVector<GMBSP_Render_Leaf> leafs;

	std::map<GMBSP_Render_BiquadraticPatch*, GMGameObject*> biquadraticPatchObjects;
	std::map<GMBSP_Render_Face*, GMGameObject*> polygonFaceObjects;
	std::map<GMBSP_Render_Face*, GMGameObject*> meshFaceObjects;
	std::map<GMBSPEntity*, GMEntityObject*> entitiyObjects;

	// 用于记录每种类型的面在faceDirectory中的位置
	Vector<GMint> polygonIndices;
	Vector<GMint> meshFaceIndices;
	Vector<GMint> patchIndices;

	BSPData* bsp = nullptr;
	Bitset facesToDraw;
	Bitset entitiesToDraw;
	GMint numPolygonFaces = 0;
	GMint numPatches = 0;
	GMint numMeshFaces = 0;
	AlignedVector<GMGameObject*> alwaysVisibleObjects;
	GMBSP_Render_VisibilityData visibilityData;

	// 用于绘制天空
	linear_math::Vector3 boundMin = BOUNDING;
	linear_math::Vector3 boundMax = -BOUNDING;
};

typedef GMBSPRenderPrivate GMBSPRenderData;
class GMBSPRender
{
	DECLARE_PRIVATE(GMBSPRender);

public:
	GMBSPRenderData& renderData();
	void generateRenderData(BSPData* bsp);
	void createObject(const GMBSP_Render_Face& face, const Shader& shader, OUT GMModel** obj);
	void createObject(const GMBSP_Render_BiquadraticPatch& biqp, const Shader& shader, OUT GMModel** obj);
	void createBox(const linear_math::Vector3& extents, const linear_math::Vector3& position, const Shader& shader, OUT GMModel** obj);

private:
	void generateVertices();
	void generateFaces();
	void generateShaders();
	void generateLightmaps();
	void generateLeafs();
	void generatePlanes();
	void generateVisibilityData();
};

END_NS
#endif