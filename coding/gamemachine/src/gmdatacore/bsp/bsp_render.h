#ifndef __BSP_RENDER_H__
#define __BSP_RENDER_H__
#include "common.h"
#include "utilities/vmath.h"
#include "utilities/bitset.h"
#include <vector>
#include <map>
#include "bsp.h"
BEGIN_NS

class GameObject;

// structs for rendering
class BSP_Render_Vertex
{
public:
	vmath::vec3 position;
	float decalS, decalT;
	float lightmapS, lightmapT;

	BSP_Render_Vertex operator+(const BSP_Render_Vertex & rhs) const
	{
		BSP_Render_Vertex result;
		result.position = position + rhs.position;
		result.decalS = decalS + rhs.decalS;
		result.decalT = decalT + rhs.decalT;
		result.lightmapS = lightmapS + rhs.lightmapS;
		result.lightmapT = lightmapT + rhs.lightmapT;

		return result;
	}

	BSP_Render_Vertex operator*(const float rhs) const
	{
		BSP_Render_Vertex result;
		result.position = position*rhs;
		result.decalS = decalS*rhs;
		result.decalT = decalT*rhs;
		result.lightmapS = lightmapS*rhs;
		result.lightmapT = lightmapT*rhs;

		return result;
	}
};

struct BSP_Render_Face
{
	int firstVertex;
	int numVertices;
	int textureIndex;
	int lightmapIndex;
	int firstIndex;
	int numIndices;
};

struct BSP_Render_FaceDirectoryEntry
{
	BSPSurfaceType faceType;
	int typeFaceNumber;		//face number in the list of faces of this type
};

//every patch (curved surface) is split into biquadratic (3x3) patches
class BSP_Render_BiquadraticPatch
{
public:
	bool tesselate(int newTesselation);

	BSP_Render_Vertex controlPoints[9];

	int tesselation;
	BSP_Render_Vertex* vertices;
	GLuint * indices;

	//arrays for multi_draw_arrays
	int * trianglesPerRow;
	unsigned int ** rowIndexPointers;

	BSP_Render_BiquadraticPatch() : vertices(NULL)
	{}
	~BSP_Render_BiquadraticPatch()
	{
		if (vertices)
			delete[] vertices;
		vertices = nullptr;

		if (indices)
			delete[] indices;
		indices = nullptr;
	}
};

//curved surface
class BSP_Render_Patch
{
public:
	int textureIndex;
	int lightmapIndex;
	int width, height;

	int numQuadraticPatches;
	BSP_Render_BiquadraticPatch* quadraticPatches;
};

enum
{
	BOUNDING = 99999,
};

class BSP_Render_Leaf
{
public:
	int cluster;	//cluster index for visdata
	vmath::vec3 boundingBoxVertices[8];
	int firstLeafFace;	//first index in leafFaces array
	int numFaces;
};

class BSP_Render_VisibilityData
{
public:
	int numClusters;
	int bytesPerCluster;
	GMbyte * bitset;

	BSP_Render_VisibilityData() : bitset(nullptr)
	{}
	~BSP_Render_VisibilityData()
	{
		if (bitset)
			delete[] bitset;
		bitset = nullptr;
	}
};

struct BSPRenderPrivate
{
	BSPRenderPrivate()
		: numPolygonFaces(0)
		, numPatches(0)
		, numMeshFaces(0)
		, boundMin(BOUNDING)
		, boundMax(-BOUNDING)
	{

	}

	BSPData* bsp;
	//data for render:
	std::vector<BSP_Render_Vertex> vertices;
	Bitset facesToDraw;
	Bitset entitiesToDraw;
	GMint numPolygonFaces;
	GMint numPatches;
	GMint numMeshFaces;
	std::vector<GameObject*> alwaysVisibleObjects;
	std::vector<BSP_Render_FaceDirectoryEntry> faceDirectory;
	std::vector<BSP_Render_Face> polygonFaces;
	std::vector<BSP_Render_Face> meshFaces;
	std::vector<BSP_Render_Patch> patches;
	std::vector<BSP_Render_Leaf> leafs;
	BSP_Render_VisibilityData visibilityData;

	std::map<BSP_Render_BiquadraticPatch*, GameObject*> biquadraticPatchObjects;
	std::map<BSP_Render_Face*, GameObject*> polygonFaceObjects;
	std::map<BSP_Render_Face*, GameObject*> meshFaceObjects;
	std::map<BSPEntity*, GameObject*> entitiyObjects;

	// 用于绘制天空
	vmath::vec3 boundMin;
	vmath::vec3 boundMax;
};

typedef BSPRenderPrivate BSPRenderData;
class Object;
struct Shader;
class BSPRender
{
	DEFINE_PRIVATE(BSPRender);

public:
	BSPRenderData& renderData();
	void generateRenderData(BSPData* bsp);
	void createObject(const BSP_Render_Face& face, const Shader& shader, OUT Object** obj);
	void createObject(const BSP_Render_BiquadraticPatch& biqp, const Shader& shader, OUT Object** obj);
	void createBox(const vmath::vec3& extents, const vmath::vec3& position, const Shader& shader, OUT Object** obj);

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