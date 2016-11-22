﻿#include "stdafx.h"
#include "objreader_private.h"
#include "utilities/scanner.h"
#include "utilities/assert.h"
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

FaceIndices::FaceIndices()
	: m_vertexIndex(NONE)
	, m_textureIndex(NONE)
	, m_normalIndex(NONE)
{

}

FaceIndices::FaceIndices(GLint vertexIndex, GLint textureIndex, GLint normalIndex)
	: m_vertexIndex(vertexIndex)
	, m_textureIndex(textureIndex)
	, m_normalIndex(normalIndex)
{
}

GLint FaceIndices::get(Which which)
{
	switch (which)
	{
	case Vertex:
		return m_vertexIndex;
	case Texture:
		return m_textureIndex;
	case Normal:
		return m_normalIndex;
	}
	ASSERT(false);
	return -1;
}

void ObjReader_Private::parseLine(const char* line)
{
	Scanner scanner(line, isWhiteSpace);
	char command[LINE_MAX];
	scanner.next(command);

	if (strEqual(command, KW_REMARK))
		return;

	if (strEqual(command, KW_VERTEX))
	{
		Ffloat v1, v2, v3;
		scanner.nextFloat(&v1);
		scanner.nextFloat(&v2);
		scanner.nextFloat(&v3);
		Vertices vertices (v1, v2, v3);
		m_vertices.push_back(vertices);
	}
	else if (strEqual(command, KW_VNORMAL))
	{
		Ffloat v1, v2, v3;
		scanner.nextFloat(&v1);
		scanner.nextFloat(&v2);
		scanner.nextFloat(&v3);
		VertexNormal normal(v1, v2, v3);
		m_normals.push_back(normal);
	}
	else if (strEqual(command, KW_VTEXTURE))
	{
		Ffloat v1, v2;
		scanner.nextFloat(&v1);
		scanner.nextFloat(&v2);
		VertexTexture texture(v1, v2, NONE);
		m_textures.push_back(texture);
	}
	else if (strEqual(command, KW_FACE))
	{
		m_pCallback->onBeginFace();
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
			FaceIndices indices(i1, i2, i3);
			m_pCallback->onDrawFace(&indices);
		}
		m_pCallback->onEndFace();
	}
	else if (strEqual(command, KW_MTLLIB))
	{
		char fn[LINE_MAX];
		scanner.nextToTheEnd(fn);
		std::string mtlfile = m_workingDir.append(fn);
		mtlReader.load(mtlfile.c_str());
	}
	else if (strEqual(command, KW_USEMTL))
	{
		char name[LINE_MAX];
		scanner.nextToTheEnd(name);
		const MaterialProperties& properties = mtlReader.getProperties(name);
		m_pCallback->onMaterial(properties);
	}
}

void ObjReader_Private::draw()
{
	m_pCallback->draw();
}

void ObjReader_Private::beginLoad()
{
	m_pCallback->onBeginLoad();
}

void ObjReader_Private::endLoad()
{
	m_pCallback->onEndLoad();
}
VectorContainer ObjReader_Private::get(DataType dataType, Fint index)
{
	VectorContainer def(NONE, NONE, NONE);
	switch (dataType)
	{
	case Vertex:
		return index == NONE ? def : m_vertices.at(index - 1);
	case Texture:
		return index == NONE ? def : m_textures.at(index - 1);
	case Normal:
		return index == NONE ? def : m_normals.at(index - 1);
	}
	ASSERT(false);
	return def;
}

void ObjReaderCallback::onBeginFace()
{
	glBegin(GL_POLYGON);
}

void ObjReaderCallback::onEndFace()
{
	glEnd();
}

void ObjReaderCallback::onMaterial(const MaterialProperties& p)
{
	Ffloat Ka[] = { p.Ka_r, p.Ka_g, p.Ka_b };
	Ffloat Kd[] = { p.Kd_r, p.Kd_g, p.Kd_b };
	Ffloat Ks[] = { p.Ks_r, p.Ks_g, p.Ks_b };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Ks);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, p.Ns);
}

void ObjReaderCallback::draw()
{
	glCallList(m_listID);
}

ObjReaderCallback::~ObjReaderCallback()
{
	glDeleteLists(m_listID, 1);
}

void ObjReaderCallback::onBeginLoad()
{
	m_listID = glGenLists(1);
	glNewList(m_listID, m_data->mode() == ObjReader_Private::LoadAndDraw ? GL_COMPILE_AND_EXECUTE : GL_COMPILE);
}

void ObjReaderCallback::onEndLoad()
{
	glEndList();
}

void ObjReaderCallback::onDrawFace(FaceIndices* faceIndices)
{
	const VectorContainer& v = m_data->get(ObjReader_Private::Vertex, faceIndices->get(FaceIndices::Vertex));
	const VectorContainer& n = m_data->get(ObjReader_Private::Normal, faceIndices->get(FaceIndices::Normal));
	glNormal3f(n.get(VectorContainer::V1), n.get(VectorContainer::V2), n.get(VectorContainer::V3));
	glVertex3f(v.get(VectorContainer::V1), v.get(VectorContainer::V2), v.get(VectorContainer::V3));
}