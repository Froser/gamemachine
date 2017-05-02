#include "stdafx.h"
#include "modelreader_obj.h"
#include "gmdatacore/object.h"
#include "gmdatacore/gamepackage.h"
#include "utilities/scanner.h"

#define RESERVED 2048
#define INVALID -1

static bool slashPredicate(char c)
{
	return c == '/';
}

class StringReader
{
public:
	StringReader(const char* string)
		: m_string(string)
		, m_p(string)
	{
	}

	bool readLine(char* line)
	{
		GMint offset = 0;
		while (*m_p != '\r' && *m_p != '\n')
		{
			if (!*m_p)
			{
				line[offset] = 0;
				return false;
			}
			line[offset++] = *(m_p++);
		}

		m_p++;
		line[offset] = 0;
		return true;
	}

private:
	const char* m_string;
	const char* m_p;
};

template <GMint D, typename T>
static void pushBackData(Scanner& scanner, std::vector<T>& container)
{
	T data;
	for (GMint i = 0; i < D; i++)
	{
		scanner.nextFloat(&(data[i]));
	}
	container.push_back(data);
}

static void pushBackData(const vmath::vec3 extents, const vmath::vec3& position, Scanner& scanner, std::vector<vmath::vec3>& container)
{
	vmath::vec3 data;
	for (GMint i = 0; i < 3; i++)
	{
		GMfloat d;
		scanner.nextFloat(&d);
		d = d * extents[i] + position[i];
		data[i] = d;
	}
	container.push_back(data);
}

ModelReader_Obj::ModelReader_Obj()
{
	D(d);
	d.object = nullptr;
	d.currentComponent = nullptr;
}

bool ModelReader_Obj::load(const vmath::vec3 extents, const vmath::vec3& position, GamePackageBuffer& buffer, OUT Object** object)
{
	D(d);
	buffer.convertToStringBuffer();
	char line[LINE_MAX];
	StringReader sr((char*)buffer.buffer);

	d.object = new Object;
	ChildObject* child = new ChildObject();
	d.object->append(child);

	// 事先分配一些内存，提高效率
	d.vertices.reserve(RESERVED);

	while (sr.readLine(line))
	{
		Scanner s(line);
		char token[LINE_MAX];
		s.next(token);

		if (strEqual(token, "#"))
		{
			// comment
		}
		else if (strEqual(token, "v"))
		{
			// vertex
			pushBackData(extents, position, s, d.vertices);
		}
		else if (strEqual(token, "vn"))
		{
			// normal
			pushBackData<3>(s, d.normals);
		}
		else if (strEqual(token, "vt"))
		{
			// texture
			pushBackData<2>(s, d.textures);
		}
		else if (strEqual(token, "mtllib"))
		{
			// material
		}
		else if (strEqual(token, "usemtl"))
		{
			// use material
			char name[LINE_MAX];
			s.next(name);
			d.currentMaterialName = name;
			d.currentComponent = nullptr;
		}
		else if (strEqual(token, "f"))
		{
			// face
			appendFace(s);
		}
	}

	*object = d.object;
	return true;
}

bool ModelReader_Obj::test(const GamePackageBuffer& buffer)
{
	return buffer.buffer[0] == '#';
}

void ModelReader_Obj::appendFace(Scanner& scanner)
{
	D(d);
	const ModelReader_Obj_Material& material = d.materials[d.currentMaterialName];

	ChildObject* child = d.object->getChildObjects()[0];
	if (!d.currentComponent)
		d.currentComponent = new Component(child);
	// setup component by material

	char face[LINE_MAX];
	d.currentComponent->beginFace();
	while (true)
	{
		scanner.next(face);
		if (!strlen(face))
			break;

		Scanner faceScanner(face, false, slashPredicate);

		GMint v = INVALID, t = INVALID, n = INVALID;
		faceScanner.nextInt(&v);
		faceScanner.nextInt(&t);
		faceScanner.nextInt(&n);

		if (v != INVALID)
		{
			auto& vec = d.vertices[v - 1];
			d.currentComponent->vertex(vec[0], vec[1], vec[2]);
		}
		if (t != INVALID)
		{
			auto& vec = d.textures[t - 1];
			d.currentComponent->uv(vec[0], vec[1]);
		}
		if (n != INVALID)
		{
			auto& vec = d.normals[n - 1];
			d.currentComponent->normal(vec[0], vec[1], vec[2]);
		}
	};
	d.currentComponent->endFace();
}