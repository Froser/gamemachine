#ifndef __OBJREADER_PRIVATE_H__
#define __OBJREADER_PRIVATE_H__
#include "common.h"
#include <string.h>
#include <vector>
#include "gl/GL.h"
#include "mtlreader.h"
#include "gmdatacore/object.h"
BEGIN_NS

struct IFactory;
class TextureContainer;
class ObjReaderPrivate
{
	friend class ObjReader;

private:
	ObjReaderPrivate();
	~ObjReaderPrivate();
	void setObject(Object* obj) { m_object = obj; }
	void setWorkingDir(const std::string& workingDir) { m_workingDir = workingDir; }
	void setFactory(IFactory* factory) { m_factory = factory; }
	void parseLine(const char* line);
	void pushMaterial();
	void pushChildObject(const char* nextChildObjectName);
	void endParse();

private:
	Object* m_object;
	std::string m_workingDir;
	std::vector<GMfloat> m_vertices;
	std::vector<GMfloat> m_normals;
	std::vector<GMfloat> m_uvs;
	MtlReader* m_pMtlReader;
	Component* m_currentComponent;
	GMuint m_vertexOffset;
	const MaterialProperties* m_currentMaterial;
	ChildObject* m_currentChildObject;
	IFactory* m_factory;
};

END_NS
#endif