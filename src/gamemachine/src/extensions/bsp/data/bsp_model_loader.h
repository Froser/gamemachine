#ifndef __BSPMODEL_LOADER_H__
#define __BSPMODEL_LOADER_H__
#include <gmcommon.h>
#include <linearmath.h>

class TiXmlDocument;
class TiXmlElement;

BEGIN_NS

GM_ALIGNED_STRUCT(Model)
{
	glm::vec3 extents;
	char classname[64];
	char model[64];
	bool create;
};

// BSP中读取出来的Entity会在这里找到对应的model，来进行绘制
typedef Map<GMString, Model*> ModelMap;

class GMBSPGameWorld;
GM_PRIVATE_OBJECT(BSPModelLoader)
{
	GMString directory;
	GMBSPGameWorld* world;
	ModelMap items;
	AlignedVector<TiXmlDocument*> modelDocs;
};

class BSPModelLoader : public GMObject
{
	DECLARE_PRIVATE(BSPModelLoader)

public:
	BSPModelLoader();
	~BSPModelLoader();

public:
	void init(const GMString& directory, GMBSPGameWorld* world);
	void load();
	Model* find(const GMString& classname);

private:
	void parse(const char* buf);
	void parseItem(TiXmlElement* ti);
};

END_NS
#endif