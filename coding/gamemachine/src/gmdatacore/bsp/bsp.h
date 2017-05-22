#ifndef __BSP_H__
#define __BSP_H__
#include "common.h"
#include <string>
#include "utilities/vector.h"
#include "utilities/utilities.h"
#include "utilities/linearmath.h"
#include "bsp_surface_flags.h"
#include "utilities/vector.h"
BEGIN_NS

// key / value pair sizes in the entities lump
#define	MAX_KEY				32
#define	MAX_VALUE			1024

// the maximum size of game relative pathnames
#define	MAX_QPATH		64

#define BSP_IDENT	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
// little-endian "IBSP"

#define BSP_VERSION			46

#define	LUMP_ENTITIES		0 // 实体
#define	LUMP_SHADERS		1 // 纹理(Textures)
#define	LUMP_PLANES			2 // 平面
#define	LUMP_NODES			3 // 节点
#define	LUMP_LEAFS			4 // 叶子
#define	LUMP_LEAFSURFACES	5 // 叶子面
#define	LUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
#define	LUMP_DRAWVERTS		10 // 顶点
#define	LUMP_DRAWINDEXES	11 // 顶点索引
#define	LUMP_FOGS			12 // 特效(Effects)
#define	LUMP_SURFACES		13 // 表面(Faces)
#define	LUMP_LIGHTMAPS		14
#define	LUMP_LIGHTGRID		15
#define	LUMP_VISIBILITY		16
#define	HEADER_LUMPS		17

// Surface type
typedef enum {
	MST_BAD,
	MST_PLANAR, //PolygonFace
	MST_PATCH, //Patch
	MST_TRIANGLE_SOUP, //Mesh
	MST_FLARE
} BSPSurfaceType;

// BSP file struct
typedef struct {
	int		fileofs, filelen;
} BSPLump;

typedef linear_math::Vector3 BSPVector3;

typedef struct {
	int			ident;
	int			version;

	BSPLump		lumps[HEADER_LUMPS];
} BSPHeader;

typedef struct {
	float		mins[3], maxs[3];
	int			firstSurface, numSurfaces;
	int			firstBrush, numBrushes;
} BSPModel;

typedef struct {
	char		shader[MAX_QPATH];
	int			surfaceFlags;
	int			contentFlags;
} BSPShader;

typedef Plane BSPPlane;

typedef struct {
	int			planeNum;
	int			children[2];
	int			mins[3];
	int			maxs[3];
} BSPNode;

typedef struct {
	int			cluster;
	int			area;

	int			mins[3];
	int			maxs[3];

	int			firstLeafSurface;
	int			numLeafSurfaces;

	int			firstLeafBrush;
	int			numLeafBrushes;
} BSPLeaf;

typedef struct {
	int			planeNum;
	int			shaderNum;
} BSPBrushSide;

typedef struct {
	int			firstSide;
	int			numSides;
	int			shaderNum;
} BSPBrush;


typedef struct BSPPair {
	struct BSPPair	*next;
	char	*key;
	char	*value;
} BSPKeyValuePair;

typedef struct {
	BSPVector3		origin;
	int			firstDrawSurf;
	BSPKeyValuePair		*epairs;
} BSPEntity;

typedef struct {
	int			shaderNum;
	int			fogNum;
	int			surfaceType;

	int			firstVert;
	int			numVerts;

	int			firstIndex;
	int			numIndexes;

	int			lightmapNum;
	int			lightmapX, lightmapY;
	int			lightmapWidth, lightmapHeight;

	BSPVector3		lightmapOrigin;
	BSPVector3		lightmapVecs[3];

	int			patchWidth;
	int			patchHeight;
} BSPSurface;

typedef struct
{
	char	filename[1024];
	char    *buffer, *script_p, *end_p;
	int     line;
} BSPScript;

typedef struct {
	char		shader[MAX_QPATH];
	int			brushNum;
	int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
} BSPFog;

typedef struct {
	BSPVector3	xyz;
	float		st[2];
	float		lightmap[2];
	BSPVector3	normal;
	GMbyte		color[4];
} BSPDrawVertices;

#define	MAXTOKEN 1024
#define	MAX_INCLUDES	8

struct BSPLightVolumes
{
	linear_math::Vector3 lightVolOrigin;
	linear_math::Vector3 lightVolSize;
	linear_math::Vector3 lightVolInverseSize;
	linear_math::Vector3 lightVolBounds;
	GMbyte* volData;
};

struct BSPPrivate
{
	friend class BSP;
	BSPPrivate()
	{
	}

	GMbyte* buffer;
	BSPHeader* header;
	AlignedVector<BSPEntity> entities;
	GMint nummodels;
	Vector<BSPModel> models;
	GMint numShaders;
	Vector<BSPShader> shaders;
	GMint entdatasize;
	Vector<char> entdata;
	GMint numleafs;
	Vector<BSPLeaf> leafs;
	GMint numplanes;
	AlignedVector<BSPPlane> planes;
	GMint numnodes;
	Vector<BSPNode> nodes;
	GMint numleafsurfaces;
	Vector<GMint> leafsurfaces;
	GMint numleafbrushes;
	Vector<GMint> leafbrushes;
	GMint numbrushes;
	Vector<BSPBrush> brushes;
	GMint numbrushsides;
	Vector<BSPBrushSide> brushsides;
	GMint numLightBytes;
	Vector<GMbyte> lightBytes;
	GMint numGridPoints;
	Vector<GMbyte> gridData;
	GMint numVisBytes;
	Vector<GMbyte> visBytes;
	GMint numDrawVertices;
	AlignedVector<BSPDrawVertices> vertices;
	GMint numDrawIndexes;
	Vector<int> drawIndexes;
	GMint numDrawSurfaces;
	AlignedVector<BSPSurface> drawSurfaces;
	GMint numFogs;
	Vector<BSPFog> fogs;

	BSPLightVolumes lightVols;

private:
	char token[MAXTOKEN];
	bool endofscript;
	bool tokenready;
	BSPScript scriptstack[MAX_INCLUDES];
	BSPScript *script;
	GMint scriptline;
};

typedef BSPPrivate BSPData;
struct GamePackageBuffer;
class BSP
{
	DEFINE_PRIVATE(BSP);

public:
	BSP();
	~BSP();

public:
	void loadBsp(const GamePackageBuffer& buf);
	BSPData& bspData();

private:
	void swapBsp();
	void toGLCoord();
	void parseFromMemory(char *buffer, int size);
	void generateLightVolumes();
	void parseEntities();
	BSPEntity* parseEntity();
	bool getToken(bool crossline);
	BSPKeyValuePair* parseEpair();
	void addScriptToStack(const char *filename);
	bool endOfScript(bool crossline);
};

END_NS
#endif
