#ifndef __BSPMODEL_LOADER_H__
#define __BSPMODEL_LOADER_H__
#include "common.h"
#include "model.h"
#include <string>
#include <vector>
#include <map>

class TiXmlDocument;
class TiXmlElement;

BEGIN_NS
// BSP中读取出来的Entity会在这里找到对应的model，来进行绘制
class BSPGameWorld;
class BSPModelLoader
{
public:
	BSPModelLoader();
	~BSPModelLoader();

public:
	void init(const char* directory, BSPGameWorld* world);
	void load();
	Model* find(const std::string& classname);

private:
	void parse(const char* data);
	void parseItem(TiXmlElement* ti);

private:
	std::string m_directory;
	BSPGameWorld* m_world;
	std::map<std::string, Model> m_items;
	std::vector<TiXmlDocument*> m_modelDocs;
};

END_NS
#endif