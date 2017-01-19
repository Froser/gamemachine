#include "stdafx.h"
#include "ObjReaderPrivate.h"
#include "utilities/scanner.h"
#include "utilities/assert.h"
#include "gmdatacore/imagereader/imagereader.h"
#include <locale>
#include "gmengine/controller/factory.h"

#define NONE 0
#define KW_REMARK "#"
#define KW_VERTEX "v"
#define KW_VNORMAL "vn"
#define KW_VTEXTURE "vt"
#define KW_FACE "f"
#define KW_MTLLIB "mtllib"
#define KW_USEMTL "usemtl"
#define KW_OBJECT "o"

static bool isSeparator(char c)
{
	return c == '/';
}

static void pushVector(std::vector<GMfloat>& to, std::vector<GMfloat>& indicesVector, GMuint indices)
{
	if (indicesVector.size() == 0)
		return;

#if _DEBUG
	to.push_back(indicesVector.at((indices - 1) * 4));
	to.push_back(indicesVector.at((indices - 1) * 4 + 1));
	to.push_back(indicesVector.at((indices - 1) * 4 + 2));
#else
	to.push_back(indicesVector[(indices - 1) * 4]);
	to.push_back(indicesVector[(indices - 1) * 4 + 1]);
	to.push_back(indicesVector[(indices - 1) * 4 + 2]);
#endif
	to.push_back(1.0f);
}

static void pushUVVector(std::vector<GMfloat>& to, std::vector<GMfloat>& indicesVector, GMuint indices)
{
	if (indicesVector.size() == 0)
		return;

	to.push_back(indicesVector.data()[(indices - 1) * 2]);
	to.push_back(indicesVector.data()[(indices - 1) * 2 + 1]);
}

static void createTexture(IFactory* factory, const char* path, OUT ITexture** texture)
{
	Image* img;
	bool b = ImageReader::load(path, &img);
	ASSERT(b);
	factory->createTexture(img, texture);
}

ObjReaderPrivate::ObjReaderPrivate()
	: m_currentComponent(nullptr)
	, m_pMtlReader(new MtlReader())
	, m_currentMaterial(nullptr)
	, m_vertexOffset(0)
	, m_factory(nullptr)
	, m_currentChildObject(nullptr)
{
}

ObjReaderPrivate::~ObjReaderPrivate()
{
	delete m_pMtlReader;
}

void ObjReaderPrivate::parseLine(const char* line)
{
	if (!m_currentChildObject)
		m_currentChildObject = new ChildObject();

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
				break;

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
			pushVector(m_currentChildObject->vertices(), m_vertices, i1);
			pushUVVector(m_currentChildObject->uvs(), m_uvs, i2);
			pushVector(m_currentChildObject->normals(), m_normals, i3);
		} while (true);

		m_currentComponent->pushBackVertexOffset(edgeCount);
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
		pushMaterial();
		char name[LINE_MAX];
		scanner.nextToTheEnd(name);
		m_currentMaterial = &m_pMtlReader->getProperties(name);
	}
	else if (strEqual(command, KW_OBJECT))
	{
		char name[LINE_MAX];
		scanner.nextToTheEnd(name);
		pushChildObject(name);
	}
}

void ObjReaderPrivate::pushMaterial()
{
	if (!m_currentComponent)
	{
		m_currentComponent = new Component();
		return;
	}

	GMuint vertexCount = m_vertexOffset - m_currentComponent->getOffset();
	if (vertexCount == 0)
		return;

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

	// 创建纹理
	GMuint textureIdx = 0;
	if (m_currentMaterial->map_Ka_switch)
	{
		ASSERT(m_factory);
		std::string texturePath = std::string(m_workingDir).append(m_currentMaterial->map_Ka);
		ITexture* texture;
		createTexture(m_factory, texturePath.c_str(), &texture);
		m.textures[textureIdx].texture = texture;
		m.textures[textureIdx].type = TextureTypeAmbient;
		m.textures[textureIdx].autorelease = 1;
		textureIdx++;
	}

	if (m_currentMaterial->map_Kd_switch)
	{
		std::string texturePath = std::string(m_workingDir).append(m_currentMaterial->map_Kd);
		ITexture* texture;
		createTexture(m_factory, texturePath.c_str(), &texture);
		m.textures[textureIdx].texture = texture;
		m.textures[textureIdx].type = TextureTypeDiffuse;
		m.textures[textureIdx].autorelease = 1;
		textureIdx++;
	}

	m_currentChildObject->appendComponent(m_currentComponent, vertexCount);

	m_currentComponent = new Component();
	m_currentComponent->setOffset(m_vertexOffset);
}

void ObjReaderPrivate::pushChildObject(const char* nextChildObjectName)
{
	if (m_vertexOffset == 0)
	{
		m_currentChildObject->setName(nextChildObjectName);
		return;
	}

	pushMaterial();
	m_vertexOffset = 0;
	m_currentComponent->setOffset(0);

	ASSERT(m_currentChildObject);
	m_object->append(m_currentChildObject);

	if (nextChildObjectName)
		m_currentChildObject = new ChildObject(nextChildObjectName);
}

void ObjReaderPrivate::endParse()
{
	pushChildObject(nullptr);
}