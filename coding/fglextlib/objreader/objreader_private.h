#ifndef __OBJREADER_PRIVATE_H__
#include "common.h"
#include <string.h>
#include <queue>
#include "gl/GL.h"
#include "containers/basic_containers.h"
#include "mtlreader.h"
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
	FaceIndices();
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
	virtual void onMaterial(const MaterialProperties& p) = 0;
};

class ObjReader_Private;
class ObjReaderCallback : public IObjReaderCallBack
{
public:
	ObjReaderCallback(ObjReader_Private* data) : m_data(data) {};
	void onDrawFace(FaceIndices* faceIndices) override;
	void onBeginFace() override;
	void onEndFace() override;
	void onMaterial(const MaterialProperties& p) override;

private:
	ObjReader_Private* m_data;
};

class ObjReaderCommand
{
public:
	enum Commands
	{
		CommandBeginFace,
		CommandDrawFace,
		CommandEndFace,
		CommandMaterial,
	};

public:
	ObjReaderCommand(Commands commands);
	ObjReaderCommand(const FaceIndices& faceIndices);
	ObjReaderCommand(const MaterialProperties& mp);

public:
	Commands type() const { return m_type; }
	FaceIndices& faceIndices() { return m_faceIndices; }
	MaterialProperties& materialProperties() { return m_materialProperties; }

private:
	Commands m_type;
	FaceIndices m_faceIndices;
	MaterialProperties m_materialProperties;
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

	enum Mode
	{
		LoadAndDraw,
		LoadOnly
	};
private:
	ObjReader_Private() { m_pCallback = new ObjReaderCallback(this); }
	~ObjReader_Private() { delete m_pCallback; }
	void setMode(int mode) { m_mode = mode; }
	void setWorkingDir(const std::string& workingDir) { m_workingDir = workingDir; }
	void parseLine(const char* line);
	void draw();
	const VectorContainer& get(DataType dataType, Fint index);

private:
	std::string m_workingDir;
	std::vector<Vertices> m_vertices;
	std::vector<VertexNormal> m_normals;
	std::vector<VertexTexture> m_textures;
	std::vector<ObjReaderCommand> m_commands;
	IObjReaderCallBack* m_pCallback;
	MtlReader mtlReader;
	int m_mode;
};

END_NS
#endif