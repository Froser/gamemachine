#ifndef __BSP_H__
#define __BSP_H__
#include "common.h"
#include <string>
#include "foundation/utilities/utilities.h"
#include "foundation/vector.h"
#include "foundation/linearmath.h"
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

typedef Plane BSPPlane;
typedef linear_math::Vector3 BSPVector3;

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

struct GMBSPPair
{
	GMBSPPair* next;
	char* key;
	char* value;
};

typedef GMBSPPair GMBSPKeyValuePair;

struct GMBSPEntity
{
	BSPVector3 origin;
	GMint firstDrawSurf;
	GMBSPKeyValuePair* epairs;
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
	linear_math::Vector3 lightVolOrigin;
	linear_math::Vector3 lightVolSize;
	linear_math::Vector3 lightVolInverseSize;
	linear_math::Vector3 lightVolBounds;
	GMbyte* volData;
};

GM_PRIVATE_OBJECT(BSP)
{
	friend class BSP;

	BSPLightVolumes lightVols;
	AlignedVector<GMBSPEntity> entities;
	AlignedVector<BSPPlane> planes;
	AlignedVector<GMBSPDrawVertices> vertices;
	AlignedVector<GMBSPSurface> drawSurfaces;
	AlignedVector<GMBSPModel> models;
	AlignedVector<GMBSPShader> shaders;
	AlignedVector<char> entdata;
	AlignedVector<GMBSPLeaf> leafs;
	AlignedVector<GMBSPNode> nodes;
	AlignedVector<GMint> leafsurfaces;
	AlignedVector<GMint> leafbrushes;
	AlignedVector<GMBSPBrush> brushes;
	AlignedVector<GMBSPBrushSide> brushsides;
	AlignedVector<GMbyte> lightBytes;
	AlignedVector<GMbyte> gridData;
	AlignedVector<GMbyte> visBytes;
	AlignedVector<GMint> drawIndexes;
	AlignedVector<GMBSPFog> fogs;

	GMint nummodels;
	GMint numShaders;
	GMint entdatasize;
	GMint numleafs;
	GMint numplanes;
	GMint numnodes;
	GMint numleafsurfaces;
	GMint numleafbrushes;
	GMint numbrushes;
	GMint numbrushsides;
	GMint numLightBytes;
	GMint numGridPoints;
	GMint numVisBytes;
	GMint numDrawVertices;
	GMint numDrawIndexes;
	GMint numDrawSurfaces;
	GMint numFogs;
	GMbyte* buffer;
	GMBSPHeader* header;

private:
	char token[MAXTOKEN];
	bool endofscript;
	bool tokenready;
	GMBSPScript scriptstack[MAX_INCLUDES];
	GMBSPScript *script;
	GMint scriptline;
};

typedef BSPPrivate BSPData;
struct GMBuffer;
GM_ALIGNED_16(class) BSP
{
	DECLARE_PRIVATE(BSP);

public:
	BSP();
	~BSP();

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
	GMBSPEntity* parseEntity();
	bool getToken(bool crossline);
	GMBSPKeyValuePair* parseEpair();
	void addScriptToStack(const char *filename);
	bool endOfScript(bool crossline);
};

END_NS
#endif
