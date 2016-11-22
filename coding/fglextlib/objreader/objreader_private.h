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

struct IObjReaderCallBack
{
	virtual void onDrawFace(FaceIndices* faceIndices) = 0;
	virtual void onBeginFace() = 0;
	virtual void onEndFace() = 0;
};

class ObjReader_Private;
class ObjReaderCallback : public IObjReaderCallBack
{
public:
	ObjReaderCallback(ObjReader_Private* data) : m_data(data) {};
	void onDrawFace(FaceIndices* faceIndices) override;
	void onBeginFace() override;
	void onEndFace() override;

private:
	ObjReader_Private* m_data;
};

class ObjReader_Private
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

private:
	ObjReader_Private() { m_pCallback = new ObjReaderCallback(this); }
	~ObjReader_Private() { delete m_pCallback; }
	void init();
	void parseLine(const char* line);
	const VectorContainer& get(DataType dataType, Fint index);

private:
	std::vector<Vertices> m_vertices;
	std::vector<VertexNormal> m_normals;
	std::vector<VertexTexture> m_textures;
	IObjReaderCallBack* m_pCallback;
};

END_NS
#endif