#ifndef __BSP_H__
#define __BSP_H__
#include "common.h"
#include <string>
#include <vector>
BEGIN_NS

// key / value pair sizes in the entities lump
#define	MAX_KEY				32
#define	MAX_VALUE			1024

// the maximum size of game relative pathnames
#define	MAX_QPATH		64

#define BSP_IDENT	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
// little-endian "IBSP"

#define BSP_VERSION			46

#define	LUMP_ENTITIES		0
#define	LUMP_SHADERS		1
#define	LUMP_PLANES			2
#define	LUMP_NODES			3
#define	LUMP_LEAFS			4
#define	LUMP_LEAFSURFACES	5
#define	LUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
#define	LUMP_DRAWVERTS		10
#define	LUMP_DRAWINDEXES	11
#define	LUMP_FOGS			12
#define	LUMP_SURFACES		13
#define	LUMP_LIGHTMAPS		14
#define	LUMP_LIGHTGRID		15
#define	LUMP_VISIBILITY		16
#define	HEADER_LUMPS		17

typedef struct {
	int		fileofs, filelen;
} BSPLump;

typedef float BSPVector3[3];

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

typedef struct {
	float		normal[3];
	float		dist;
} BSPPlane;

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
	struct bspbrush_s	*brushes;
	struct parseMesh_s	*patches;
	int			firstDrawSurf;
	BSPKeyValuePair		*epairs;
} BSPEntity;

typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE
} BSPMapSurface;

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

#define	MAXTOKEN 1024
#define	MAX_INCLUDES	8

struct BSPPrivate
{
	void* buffer;
	std::string filename;
	BSPHeader* header;
	GMint numentities;
	std::vector<BSPEntity> entities;
	GMint nummodels;
	std::vector<BSPModel> models;
	GMint numShaders;
	std::vector<BSPShader> shaders;
	GMint dentdatasize;
	std::vector<char> dentdata;
	GMint numleafs;
	std::vector<BSPLeaf> leafs;
	GMint numplanes;
	std::vector<BSPPlane> planes;
	GMint numnodes;
	std::vector<BSPNode> nodes;
	GMint numleafsurfaces;
	std::vector<int> leafsurfaces;
	GMint numleafbrushes;
	std::vector<int> leafbrushes;
	GMint numbrushes;
	std::vector<BSPBrush> brushes;
	GMint numbrushsides;
	std::vector<BSPBrushSide> brushsides;
	GMint numLightBytes;
	std::vector<unsigned char> lightBytes;
	GMint numGridPoints;
	std::vector<unsigned char> gridData;
	GMint numVisBytes;
	std::vector<unsigned char> visBytes;
	GMint numDrawIndexes;
	std::vector<int> drawIndexes;
	GMint numDrawSurfaces;
	std::vector<BSPSurface> drawSurfaces;

	BSPScript scriptstack[MAX_INCLUDES];
	BSPScript *script;
	GMint scriptline;

	char token[MAXTOKEN];
	bool endofscript;
	bool tokenready;
};

typedef BSPPrivate BSPData;

class BSP
{
	DEFINE_PRIVATE(BSP);

public:
	BSP();
	~BSP();

public:
	void loadBsp(const char* filename);
	const BSPData& bspData();
	bool findVectorByName(float* outvec, const char* name);

private:
	void readFile();
	void swapBsp();
	void parseFromMemory(char *buffer, int size);
	void parseEntities();
	bool parseEntity();
	bool getToken(bool crossline);
	BSPKeyValuePair* parseEpair();
	void addScriptToStack(const char *filename);
	bool endOfScript(bool crossline);
	const char* valueForKey(const BSPEntity *ent, const char *key);
	bool vectorForKey(const BSPEntity *ent, const char *key, BSPVector3 vec);
};

END_NS
#endif