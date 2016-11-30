#include "stdafx.h"
#include "ObjReaderPrivate.h"
#include "utilities/scanner.h"
#include "utilities/assert.h"
#include "imagereader/imagereader.h"
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
		Vertices vertices (v1, v2, v3);
		m_vertices.push_back(vertices);
	}
	else if (strEqual(command, KW_VNORMAL))
	{
		GMfloat v1, v2, v3;
		scanner.nextFloat(&v1);
		scanner.nextFloat(&v2);
		scanner.nextFloat(&v3);
		VertexNormal normal(v1, v2, v3);
		m_normals.push_back(normal);
	}
	else if (strEqual(command, KW_VTEXTURE))
	{
		GMfloat v1, v2;
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
		std::string mtlfile = m_workingDir;
		mtlfile.append(fn);
		mtlReader->load(mtlfile.c_str());
	}
	else if (strEqual(command, KW_USEMTL))
	{
		char name[LINE_MAX];
		scanner.nextToTheEnd(name);
		const MaterialProperties& properties = mtlReader->getProperties(name);
		m_pCallback->onMaterial(properties);
	}
}

void ObjReaderPrivate::draw()
{
	m_pCallback->draw();
}

void ObjReaderPrivate::beginLoad()
{
	m_pCallback->onBeginLoad();
}

void ObjReaderPrivate::endLoad()
{
	m_pCallback->onEndLoad();
}

VectorContainer ObjReaderPrivate::get(DataType dataType, GMint index)
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

//////////////////////////////////////////////////////////////////////////
// Callback:
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
	if (p.hasTexture)
	{
		glBindTexture(GL_TEXTURE_2D, p.textureID);
	}

	if (p.Ka_switch)
	{
		GMfloat Ka[] = { p.Ka_r, p.Ka_g, p.Ka_b };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ka);
	}

	if (p.Kd_switch)
	{
		GMfloat Kd[] = { p.Kd_r, p.Kd_g, p.Kd_b };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Kd);
	}

	if (p.Ks_switch)
	{
		GMfloat Ks[] = { p.Ks_r, p.Ks_g, p.Ks_b };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Ks);
	}

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, p.Ns);
}

void ObjReaderCallback::onAddTexture(Image* in, GMuint* textureIDOut)
{
	glGenTextures(1, textureIDOut);
	glBindTexture(GL_TEXTURE_2D, *textureIDOut);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, in->getWidth(), in->getHeight(), 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, in->asTexture());
}

ObjReaderPrivate::ObjReaderPrivate()
{
	m_pCallback = new ObjReaderCallback(this);
	mtlReader = new MtlReader();
	mtlReader->setCallback(m_pCallback);
}

ObjReaderPrivate::~ObjReaderPrivate()
{
	delete mtlReader;
	delete m_pCallback;
}

void ObjReaderCallback::onRemoveTexture(GMuint textureIDOut)
{
	GMuint id[] = { textureIDOut };
	glDeleteTextures(1, id);
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
	glNewList(m_listID, m_data->mode() == ObjReaderPrivate::LoadAndDraw ? GL_COMPILE_AND_EXECUTE : GL_COMPILE);
}

void ObjReaderCallback::onEndLoad()
{
	glEndList();
}

void ObjReaderCallback::onDrawFace(FaceIndices* faceIndices)
{
	if (faceIndices->get(FaceIndices::Texture) != NONE)
	{
		const VectorContainer& t = m_data->get(ObjReaderPrivate::Texture, faceIndices->get(FaceIndices::Texture));
		glTexCoord2f(t.get(VectorContainer::V1), t.get(VectorContainer::V2));
	}

	if (faceIndices->get(FaceIndices::Normal) != NONE)
	{
		const VectorContainer& n = m_data->get(ObjReaderPrivate::Normal, faceIndices->get(FaceIndices::Normal));
		glNormal3f(n.get(VectorContainer::V1), n.get(VectorContainer::V2), n.get(VectorContainer::V3));
	}

	const VectorContainer& v = m_data->get(ObjReaderPrivate::Vertex, faceIndices->get(FaceIndices::Vertex));
	glVertex3f(v.get(VectorContainer::V1), v.get(VectorContainer::V2), v.get(VectorContainer::V3));
}