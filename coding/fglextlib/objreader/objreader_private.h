﻿#ifndef __OBJREADER_PRIVATE_H__
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
	virtual void onBeginLoad() = 0;
	virtual void onEndLoad() = 0;
	virtual void onDrawFace(FaceIndices* faceIndices) = 0;
	virtual void onBeginFace() = 0;
	virtual void onEndFace() = 0;
	virtual void onMaterial(const MaterialProperties& p) = 0;
	virtual void draw() = 0;
};

class ObjReader_Private;
class ObjReaderCallback : public IObjReaderCallBack
{
public:
	ObjReaderCallback(ObjReader_Private* data) : m_data(data), m_listID(-1) {};
	~ObjReaderCallback();
	void onBeginLoad() override;
	void onEndLoad() override;
	void onDrawFace(FaceIndices* faceIndices) override;
	void onBeginFace() override;
	void onEndFace() override;
	void onMaterial(const MaterialProperties& p) override;
	void draw() override;

private:
	ObjReader_Private* m_data;
	int m_listID;
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
	ObjReader_Private() : m_pCallback (new ObjReaderCallback(this)) {}
	~ObjReader_Private() { delete m_pCallback; }
	void setMode(int mode) { m_mode = mode; }
	int mode() { return m_mode; }
	void setWorkingDir(const std::string& workingDir) { m_workingDir = workingDir; }
	void parseLine(const char* line);
	void draw();
	void beginLoad();
	void endLoad();
	VectorContainer get(DataType dataType, Fint index);

private:
	std::string m_workingDir;
	std::vector<Vertices> m_vertices;
	std::vector<VertexNormal> m_normals;
	std::vector<VertexTexture> m_textures;
	IObjReaderCallBack* m_pCallback;
	MtlReader mtlReader;
	int m_mode;
};

END_NS
#endif