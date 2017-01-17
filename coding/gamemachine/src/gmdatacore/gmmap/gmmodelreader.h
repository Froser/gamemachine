#ifndef __GMMODELREADER_H__
#define __GMMODELREADER_H__
#include "common.h"
BEGIN_NS

struct IFactory;
class Object;
class GMModelReader
{
public:
	GMModelReader(IFactory* factory, const char* workingDir);

public:
	void loadModel(const char* filename, OUT Object** object);
	IFactory* getFactory() { return m_factory; }
	const char* getWorkingDir() { return m_workingDir; }

private:
	const char* m_workingDir;
	IFactory* m_factory;
};

END_NS
#endif