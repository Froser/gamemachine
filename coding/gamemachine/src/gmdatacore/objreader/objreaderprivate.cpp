#include "stdafx.h"
#include "ObjReaderPrivate.h"
#include "utilities/scanner.h"
#include "utilities/assert.h"
#include "gmdatacore/imagereader/imagereader.h"
#include <locale>

#define MAX_VERTICES_IN_ONE_FACE 4
#define NONE 0
#define KW_REMARK "#"
#define KW_VERTEX "v"
#define KW_VNORMAL "vn"
#define KW_VTEXTURE "vt"
#define KW_FACE "f"
#define KW_MTLLIB "mtllib"
#define KW_USEMTL "usemtl"

static bool strEqual(const char* str1, const char* str2)
{
	return !strcmp(str1, str2);
}

static bool isWhiteSpace(char c)
{
	return !!isspace(c);
}

static bool isSeparator(char c)
{
	return c == '/';
}

ObjReaderPrivate::ObjReaderPrivate()
	: m_currentComponent(new Component())
	, m_pMtlReader(new MtlReader())
	, m_currentMaterial(nullptr)
{
}

ObjReaderPrivate::~ObjReaderPrivate()
{
	delete m_pMtlReader;
}

void ObjReaderPrivate::parseLine(const char* line)
{
	Scanner scanner(line, isWhiteSpace);
	char command[LINE_MAX];
	scanner.next(command);

	if (strEqual(command, KW_REMARK))
		return;

	if (strEqual(command, KW_VERTEX))
	{
		GMfloat v1, v2, v3;
		scanner.nextFloat(&v1);
		scanner.nextFloat(&v2);
		scanner.nextFloat(&v3);
		m_vertices.push_back(v1);
		m_vertices.push_back(v2);
		m_vertices.push_back(v3);
		m_vertices.push_back(1.0f);
	}
	else if (strEqual(command, KW_VNORMAL))
	{
		GMfloat v1, v2, v3;
		scanner.nextFloat(&v1);
		scanner.nextFloat(&v2);
		scanner.nextFloat(&v3);
		m_normals.push_back(v1);
		m_normals.push_back(v2);
		m_normals.push_back(v3);
		m_normals.push_back(1.0f);
	}
	else if (strEqual(command, KW_VTEXTURE))
	{
		GMfloat v1, v2;
		scanner.nextFloat(&v1);
		scanner.nextFloat(&v2);
		//VertexTexture texture(v1, v2, NONE);
		//m_textures.push_back(texture);
	}
	else if (strEqual(command, KW_FACE))
	{
		for (int i = 0; i < MAX_VERTICES_IN_ONE_FACE; i++)
		{
			char subCmd[LINE_MAX];
			scanner.next(subCmd);
			if (strlen(subCmd) == 0)
				break;

			Scanner faceScanner(subCmd, false, isSeparator);

			GLint i1, i2, i3;
			if (!faceScanner.nextInt(&i1))
				i1 = NONE;
			if (!faceScanner.nextInt(&i2))
				i2 = NONE;
			if (!faceScanner.nextInt(&i3))
				i3 = NONE;
			m_indices.push_back(i1);
		}
		m_indices.push_back(RESTART_INDEX);
	}
	else if (strEqual(command, KW_MTLLIB))
	{
		char fn[LINE_MAX];
		scanner.nextToTheEnd(fn);
		std::string mtlfile = m_workingDir;
		mtlfile.append(fn);
		m_pMtlReader->load(mtlfile.c_str());
	}
	else if (strEqual(command, KW_USEMTL))
	{
		pushData();

		char name[LINE_MAX];
		scanner.nextToTheEnd(name);
		m_currentMaterial = &m_pMtlReader->getProperties(name);
	}
}

void ObjReaderPrivate::pushData()
{
	GMuint size = m_indices.size();
	if (size == 0)
		return;

	Material& m = m_currentComponent->getMaterial();
	m.Ka[0] = m_currentMaterial->Ka_r;
	m.Ka[1] = m_currentMaterial->Ka_g;
	m.Ka[2] = m_currentMaterial->Ka_b;
	m_object->appendComponent(m_currentComponent, size - m_currentComponent->getOffset());

	m_currentComponent = new Component();
	m_currentComponent->setOffset(size);
}

void ObjReaderPrivate::endParse()
{
	pushData();

	// 顶点
	{
		GMuint size = m_vertices.size();
		GMfloat* vertices = new GMfloat[size];
		GMuint cnt = 0;
		for (auto iter = m_vertices.cbegin(); iter != m_vertices.cend(); iter++, cnt++)
		{
			vertices[cnt] = *iter;
		}
		ArrayData<GMfloat> vao = { vertices, size };
		m_object->setVertices(vao);
	}

	// 法向量
	{
		GMuint size = m_normals.size();
		GMfloat* ns = new GMfloat[size];
		GMuint cnt = 0;
		for (auto iter = m_normals.cbegin(); iter != m_normals.cend(); iter++, cnt++)
		{
			ns[cnt] = *iter;
		}
		ArrayData<GMfloat> normals = { ns, size };
		m_object->setNormals(normals);
	}

	// 索引
	{
		GMuint size = m_indices.size();
		GMuint* indices = new GMuint[size];
		GMuint cnt = 0;
		for (auto iter = m_indices.begin(); iter != m_indices.end(); iter++, cnt++)
		{
			indices[cnt] = (*iter - 1);
		}
		ArrayData<GMuint> ebo = { indices, size };
		m_object->setIndices(ebo);
	}
}