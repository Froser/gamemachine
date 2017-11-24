#ifndef __BSP_H__
#define __BSP_H__
#include <gmcommon.h>
#include <string>
#include <utilities.h>
#include <linearmath.h>
#include "gmbsp_surface_flags.h"
BEGIN_NS

// key / value pair sizes in the entities lump
#define	MAX_KEY				32
#define	MAX_VALUE			1024
#define MAX_SHADER_PATH		64

#define BSP_IDENT	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
// little-endian "IBSP"

#define BSP_VERSION			46

enum
{
	LUMP_ENTITIES		=	0 , // 实体
	LUMP_SHADERS		=	1 , // 纹理(Textures)
	LUMP_PLANES			=	2 , // 平面
	LUMP_NODES			=	3 , // 节点
	LUMP_LEAFS			=	4 , // 叶子
	LUMP_LEAFSURFACES	=	5 , // 叶子面
	LUMP_LEAFBRUSHES	=	6 , 
	LUMP_MODELS			=	7 , 
	LUMP_BRUSHES		=	8 , 
	LUMP_BRUSHSIDES		=	9 , 
	LUMP_DRAWVERTS		=	10,  // 顶点
	LUMP_DRAWINDEXES	=	11,  // 顶点索引
	LUMP_FOGS			=	12,  // 特效(Effects)
	LUMP_SURFACES		=	13,  // 表面(Faces)
	LUMP_LIGHTMAPS		=	14, 
	LUMP_LIGHTGRID		=	15, 
	LUMP_VISIBILITY		=	16, 
	HEADER_LUMPS		=	17, 
};

// Surface type
enum GMBSPSurfaceType 
{
	MST_BAD,
	MST_PLANAR, //PolygonFace
	MST_PATCH, //Patch
	MST_TRIANGLE_SOUP, //Mesh
	MST_FLARE
};

typedef GMPlane BSPPlane;
typedef glm::vec3 BSPVector3;

// BSP file struct
struct GMBSPLump
{
	GMint fileofs, filelen;
};

struct GMBSPHeader
{
	GMint ident;
	GMint version;
	GMBSPLump lumps[HEADER_LUMPS];
};

struct GMBSPModel
{
	GMfloat mins[3], maxs[3];
	GMint firstSurface, numSurfaces;
	GMint firstBrush, numBrushes;
};

struct GMBSPShader
{
	char shader[MAX_SHADER_PATH];
	GMint surfaceFlags;
	GMint contentFlags;
};

struct GMBSPNode
{
	GMint planeNum;
	GMint children[2];
	GMint mins[3];
	GMint maxs[3];
};

struct GMBSPLeaf
{
	GMint cluster;
	GMint area;

	GMint mins[3];
	GMint maxs[3];

	GMint firstLeafSurface;
	GMint numLeafSurfaces;

	GMint firstLeafBrush;
	GMint numLeafBrushes;
};

struct GMBSPBrushSide
{
	GMint planeNum;
	GMint shaderNum;
};

struct GMBSPBrush
{
	GMint firstSide;
	GMint numSides;
	GMint shaderNum;
};

GM_ALIGNED_STRUCT(GMBSPEPair)
{
	GMBSPEPair* next = nullptr;
	GMString key;
	GMString value;
};

GM_ALIGNED_STRUCT(GMBSPEntity)
{
	~GMBSPEntity()
	{
		remove(epairs);
	}

	BSPVector3 origin;
	GMint firstDrawSurf = 0;
	GMBSPEPair* epairs = 0;

private:
	void remove(GMBSPEPair* node)
	{
		if (node->next)
			remove(node->next);
		delete node;
	}
};

struct GMBSPSurface
{
	GMint shaderNum;
	GMint fogNum;
	GMint surfaceType;

	GMint firstVert;
	GMint numVerts;

	GMint firstIndex;
	GMint numIndexes;

	GMint lightmapNum;
	GMint lightmapX, lightmapY;
	GMint lightmapWidth, lightmapHeight;

	BSPVector3 lightmapOrigin;
	BSPVector3 lightmapVecs[3];

	GMint patchWidth;
	GMint patchHeight;
};

struct GMBSPScript
{
	char filename[1024];
	char *buffer, *script_p, *end_p;
	GMint line;
};

struct GMBSPFog
{
	char shader[MAX_SHADER_PATH];
	GMint brushNum;
	GMint visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
};

struct GMBSPDrawVertices
{
	BSPVector3 xyz;
	GMfloat st[2];
	GMfloat lightmap[2];
	BSPVector3 normal;
	GMbyte color[4];
};

#define	MAXTOKEN 1024
#define	MAX_INCLUDES	8

GM_ALIGNED_STRUCT(BSPLightVolumes)
{
	glm::vec3 lightVolOrigin;
	glm::vec3 lightVolSize;
	glm::vec3 lightVolInverseSize;
	glm::vec3 lightVolBounds;
	GMbyte* volData;
};

GM_PRIVATE_OBJECT(GMBSP)
{
	friend class GMBSP;

	BSPLightVolumes lightVols;
	AlignedVector<BSPPlane> planes;
	AlignedVector<GMBSPDrawVertices> vertices;
	AlignedVector<GMBSPSurface> drawSurfaces;

	Vector<GMBSPModel> models;
	Vector<GMBSPShader> shaders;
	Vector<char> entdata;
	Vector<GMBSPLeaf> leafs;
	Vector<GMBSPNode> nodes;
	Vector<GMint> leafsurfaces;
	Vector<GMint> leafbrushes;
	Vector<GMBSPBrush> brushes;
	Vector<GMBSPBrushSide> brushsides;
	Vector<GMbyte> lightBytes;
	Vector<GMbyte> gridData;
	Vector<GMbyte> visBytes;
	Vector<GMint> drawIndexes;
	Vector<GMBSPFog> fogs;
	Vector<GMBSPEntity*> entities;

	GMint nummodels = 0;
	GMint numShaders = 0;
	GMint entdatasize = 0;
	GMint numleafs = 0;
	GMint numplanes = 0;
	GMint numnodes = 0;
	GMint numleafsurfaces = 0;
	GMint numleafbrushes = 0;
	GMint numbrushes = 0;
	GMint numbrushsides = 0;
	GMint numLightBytes = 0;
	GMint numGridPoints = 0;
	GMint numVisBytes = 0;
	GMint numDrawVertices = 0;
	GMint numDrawIndexes = 0;
	GMint numDrawSurfaces = 0;
	GMint numFogs = 0;
	GMbyte* buffer = nullptr;
	GMBSPHeader* header = nullptr;

private:
	char token[MAXTOKEN];
	bool endofscript;
	bool tokenready;
	GMBSPScript scriptstack[MAX_INCLUDES];
	GMBSPScript *script;
	GMint scriptline;
};

typedef GMBSPPrivate BSPData;
struct GMBuffer;
class GMBSP : public GMObject
{
	DECLARE_PRIVATE(GMBSP);

public:
	GMBSP();
	~GMBSP();

public:
	void loadBsp(const GMBuffer& buf);
	BSPData& bspData();

private:
	void loadPlanes();
	void loadVertices();
	void loadDrawSurfaces();
	void loadNoAlignData();

private:
	void swapBsp();
	void toGLCoord();
	void parseFromMemory(char *buffer, GMint size);
	void generateLightVolumes();
	void parseEntities();
	bool parseEntity(OUT GMBSPEntity** entity);
	bool getToken(bool crossline);
	GMBSPEPair* parseEpair();
	void addScriptToStack(const char *filename);
	bool endOfScript(bool crossline);
};

END_NS
#endif
