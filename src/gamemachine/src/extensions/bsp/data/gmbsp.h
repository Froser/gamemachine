#ifndef __BSP_H__
#define __BSP_H__
#include <gmcommon.h>
#include <string>
#include <gmtools.h>
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
typedef GMVec3 BSPVector3;

// BSP file struct
struct GMBSPLump
{
	GMint32 fileofs, filelen;
};

struct GMBSPHeader
{
	GMint32 ident;
	GMint32 version;
	GMBSPLump lumps[HEADER_LUMPS];
};

struct GMBSPModel
{
	GMfloat mins[3], maxs[3];
	GMint32 firstSurface, numSurfaces;
	GMint32 firstBrush, numBrushes;
};

struct GMBSPShader
{
	char shader[MAX_SHADER_PATH];
	GMint32 surfaceFlags;
	GMint32 contentFlags;
};

struct GMBSPNode
{
	GMint32 planeNum;
	GMint32 children[2];
	GMint32 mins[3];
	GMint32 maxs[3];
};

struct GMBSPLeaf
{
	GMint32 cluster;
	GMint32 area;

	GMint32 mins[3];
	GMint32 maxs[3];

	GMint32 firstLeafSurface;
	GMint32 numLeafSurfaces;

	GMint32 firstLeafBrush;
	GMint32 numLeafBrushes;
};

struct GMBSPBrushSide
{
	GMint32 planeNum;
	GMint32 shaderNum;
};

struct GMBSPBrush
{
	GMint32 firstSide;
	GMint32 numSides;
	GMint32 shaderNum;
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

	GMfloat origin[3];
	GMint32 firstDrawSurf = 0;
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
	GMint32 shaderNum;
	GMint32 fogNum;
	GMint32 surfaceType;

	GMint32 firstVert;
	GMint32 numVerts;

	GMint32 firstIndex;
	GMint32 numIndexes;

	GMint32 lightmapNum;
	GMint32 lightmapX, lightmapY;
	GMint32 lightmapWidth, lightmapHeight;

	BSPVector3 lightmapOrigin;
	BSPVector3 lightmapVecs[3];

	GMint32 patchWidth;
	GMint32 patchHeight;
};

struct GMBSPScript
{
	char filename[1024];
	char *buffer, *script_p, *end_p;
	GMint32 line;
};

struct GMBSPFog
{
	char shader[MAX_SHADER_PATH];
	GMint32 brushNum;
	GMint32 visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
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
	GMfloat lightVolOrigin[3];
	GMfloat lightVolSize[3];
	GMfloat lightVolInverseSize[3];
	GMfloat lightVolBounds[3];
	GMbyte* volData;
};

GM_PRIVATE_OBJECT_ALIGNED(GMBSP)
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
	Vector<GMint32> leafsurfaces;
	Vector<GMint32> leafbrushes;
	Vector<GMBSPBrush> brushes;
	Vector<GMBSPBrushSide> brushsides;
	Vector<GMbyte> lightBytes;
	Vector<GMbyte> gridData;
	Vector<GMbyte> visBytes;
	Vector<GMint32> drawIndexes;
	Vector<GMBSPFog> fogs;
	Vector<GMBSPEntity*> entities;

	GMint32 nummodels = 0;
	GMint32 numShaders = 0;
	GMint32 entdatasize = 0;
	GMint32 numleafs = 0;
	GMint32 numplanes = 0;
	GMint32 numnodes = 0;
	GMint32 numleafsurfaces = 0;
	GMint32 numleafbrushes = 0;
	GMint32 numbrushes = 0;
	GMint32 numbrushsides = 0;
	GMint32 numLightBytes = 0;
	GMint32 numGridPoints = 0;
	GMint32 numVisBytes = 0;
	GMint32 numDrawVertices = 0;
	GMint32 numDrawIndexes = 0;
	GMint32 numDrawSurfaces = 0;
	GMint32 numFogs = 0;
	const GMBuffer* buffer = nullptr;
	GMBSPHeader* header = nullptr;

private:
	char token[MAXTOKEN];
	bool endofscript;
	bool tokenready;
	GMBSPScript scriptstack[MAX_INCLUDES];
	GMBSPScript *script;
	GMint32 scriptline;
};

typedef GMBSPPrivate GMBSPData;
class GMBSP : public GMObject
{
	GM_DECLARE_PRIVATE(GMBSP);

public:
	GMBSP();
	~GMBSP();

public:
	void loadBsp(const GMBuffer& buf);
	GMBSPData& bspData();

private:
	void loadPlanes();
	void loadVertices();
	void loadDrawSurfaces();
	void loadNoAlignData();

private:
	void swapBsp();
	void toDxCoord();
	void parseFromMemory(char *buffer, GMint32 size);
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
