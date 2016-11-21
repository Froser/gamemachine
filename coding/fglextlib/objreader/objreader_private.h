#ifndef __OBJREADER_PRIVATE_H__
#include "common.h"
#include <vector>
#include "gl/GL.h"
#include "containers/basic_containers.h"

BEGIN_NS

class FaceIndices
{
public:
	enum Which
	{
		Vertex,
		Texture,
		Normal,
	};

public:
	FaceIndices(GLint vertexIndex, GLint textureIndex, GLint normalIndex);
	GLint get(Which);

private:
	GLint m_vertexIndex;
	GLint m_textureIndex;
	GLint m_normalIndex;
};

class ObjReader_Private
{
	friend class ObjReader;

private:
	ObjReader_Private() {}
	void init();
	void parseLine(const char* line);

private:
	std::vector<Vertices> m_vertices;
	std::vector<VertexNormal> m_normals;
	std::vector<std::vector<FaceIndices> > m_faces;
};

END_NS
#endif