#ifndef __BSPMODEL_LOADER_H__
#define __BSPMODEL_LOADER_H__
#include "common.h"
#include "model.h"
#include <string>
#include "foundation/vector.h"
#include <map>

class TiXmlDocument;
class TiXmlElement;

BEGIN_NS
// BSP中读取出来的Entity会在这里找到对应的model，来进行绘制
typedef std::map<GMString, Model*> ModelMap;

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