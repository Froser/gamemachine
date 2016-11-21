#include "stdafx.h"
#include "objreader_private.h"
#include "utilities/scanner.h"
#include <locale>

#define MAX_VERTICES_IN_ONE_FACE 4
#define NONE 0
#define KW_REMARK "#"
#define KW_VERTEX "v"
#define KW_VNORMAL "vn"
#define KW_FACE "f"

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
	return -1;
}

void ObjReader_Private::init()
{

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

	if (strEqual(command, KW_VNORMAL))
	{
		Ffloat v1, v2, v3;
		scanner.nextFloat(&v1);
		scanner.nextFloat(&v2);
		scanner.nextFloat(&v3);
		VertexNormal normal(v1, v2, v3);
		m_normals.push_back(normal);
	}

	if (strEqual(command, KW_FACE))
	{
		std::vector<FaceIndices> face;
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
			face.push_back(indices);
		}
		m_faces.push_back(face);
	}
}