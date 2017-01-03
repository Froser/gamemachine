#include "stdafx.h"
#include "ObjReaderPrivate.h"
#include "utilities/scanner.h"
#include "utilities/assert.h"
#include "gmdatacore/imagereader/imagereader.h"
#include <locale>

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

static bool isSeparator(char c)
{
	return c == '/';
}

static void pushVector(std::vector<GMfloat>& to, std::vector<GMfloat>& indicesVector, GMuint indices)
{
	if (indicesVector.size() == 0)
		return;

	to.push_back(indicesVector.data()[(indices - 1) * 4]);
	to.push_back(indicesVector.data()[(indices - 1) * 4 + 1]);
	to.push_back(indicesVector.data()[(indices - 1) * 4 + 2]);
	to.push_back(1.0f);
}

static void pushUVVector(std::vector<GMfloat>& to, std::vector<GMfloat>& indicesVector, GMuint indices)
{
	if (indicesVector.size() == 0)
		return;

	to.push_back(indicesVector.data()[(indices - 1) * 4]);
	to.push_back(indicesVector.data()[(indices - 1) * 4 + 1]);
}

ObjReaderPrivate::ObjReaderPrivate()
	: m_currentComponent(new Component())
	, m_pMtlReader(new MtlReader())
	, m_currentMaterial(nullptr)
	, m_vertexOffset(0)
{
}

ObjReaderPrivate::~ObjReaderPrivate()
{
	delete m_pMtlReader;
}

void ObjReaderPrivate::parseLine(const char* line)
{
	Scanner scanner(line);
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
		m_uvs.push_back(v1);
		m_uvs.push_back(v2);
	}
	else if (strEqual(command, KW_FACE))
	{
		GMuint edgeCount = 0;
		do
		{
			char subCmd[LINE_MAX];
			scanner.next(subCmd);
			if (strlen(subCmd) == 0)
			{
				m_currentComponent->setEdgeCountPerPolygon(edgeCount);
				break;
			}

			edgeCount++;
			Scanner faceScanner(subCmd, false, isSeparator);

			GLint i1, i2, i3;
			if (!faceScanner.nextInt(&i1))
				i1 = NONE;
			if (!faceScanner.nextInt(&i2))
				i2 = NONE;
			if (!faceScanner.nextInt(&i3))
				i3 = NONE;

			m_vertexOffset++;
			pushVector(m_object->vertices(), m_vertices, i1);
			pushUVVector(m_object->uvs(), m_uvs, i2);
			pushVector(m_object->normals(), m_normals, i3);
		} while (true);
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
	if (m_vertexOffset == 0)
		return;

	if (m_currentMaterial)
	{
		Material& m = m_currentComponent->getMaterial();
		m.Ka[0] = m_currentMaterial->Ka_r;
		m.Ka[1] = m_currentMaterial->Ka_g;
		m.Ka[2] = m_currentMaterial->Ka_b;
		m.Kd[0] = m_currentMaterial->Kd_r;
		m.Kd[1] = m_currentMaterial->Kd_g;
		m.Kd[2] = m_currentMaterial->Kd_b;
		m.Ks[0] = m_currentMaterial->Ks_r;
		m.Ks[1] = m_currentMaterial->Ks_g;
		m.Ks[2] = m_currentMaterial->Ks_b;
		m.shininess = m_currentMaterial->Ns;
	}
	m_object->appendComponent(m_currentComponent, m_vertexOffset - m_currentComponent->getOffset());

	m_currentComponent = new Component();
	m_currentComponent->setOffset(m_vertexOffset);
}

void ObjReaderPrivate::endParse()
{
	pushData();
}