#ifndef __OBJREADER_PRIVATE_H__
#define __OBJREADER_PRIVATE_H__
#include "common.h"
#include <string.h>
#include <queue>
#include "gl/GL.h"
#include "mtlreader.h"
#include "gmdatacore/object.h"
BEGIN_NS

class Image;

class ObjReaderPrivate
{
	friend class ObjReader;
	friend class ObjReaderCallback;

private:
	enum DataType
	{
		Vertex,
		Texture,
		Normal,
	};

	enum Mode
	{
		LoadAndDraw,
		LoadOnly
	};
private:
	ObjReaderPrivate();
	~ObjReaderPrivate();
	void setMode(int mode) { m_mode = mode; }
	int mode() { return m_mode; }
	void setWorkingDir(const std::string& workingDir) { m_workingDir = workingDir; }
	void parseLine(const char* line);
	void writeData(Object* obj);

private:
	Object* m_object;
	std::string m_workingDir;
	std::vector<GMfloat> m_vertices;
	std::vector<GMuint> m_indices;
	//std::vector<VertexNormal> m_normals;
	//std::vector<VertexTexture> m_textures;
	//std::vector<FaceIndices> m_indices;
	MtlReader* m_pMtlReader;
	int m_mode;
};

END_NS
#endif