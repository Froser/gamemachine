#include "stdafx.h"
#include "gmmodelreader_obj.h"
#include "gmdata/gmmodel.h"
#include "gmdata/gamepackage/gmgamepackage.h"
#include "foundation/utilities/tools.h"
#include "foundation/gamemachine.h"

#define RESERVED 4096
#define INVALID -1

namespace
{
	inline bool slashPredicate(GMwchar c)
	{
		return c == L'/';
	}

	bool isNotReturn(char p)
	{
		return p != '\r' && p != '\n';
	}

	bool isNotReturn(GMwchar p)
	{
		return p != L'\r' && p != L'\n';
	}
}

template <typename CharType>
class StringReader
{
public:
	StringReader(const CharType* string)
		: m_p(string)
	{
	}

	bool readLine(GMString& line)
	{
		line.clear();
		GMint offset = 0;
		while (isNotReturn(*m_p))
		{
			if (!*m_p)
				return false;
			line += *(m_p++);
		}

		m_p++;
		return true;
	}

private:
	const CharType* m_p;
};

template <GMint D, typename T>
static void pushBackData(GMScanner& scanner, AlignedVector<T>& container)
{
	T data;
	GMfloat f;
	GMFloat4 f4;
	for (GMint i = 0; i < D; i++)
	{
		scanner.nextFloat(&f);
		f4[i] = f;
	}
	data.setFloat4(f4);
	container.push_back(data);
}

GMModelReader_Obj::GMModelReader_Obj()
{
	init();
}

GMModelReader_Obj::~GMModelReader_Obj()
{

}

void GMModelReader_Obj::init()
{
	D(d);
	d->models = nullptr;
	d->currentModel = nullptr;
	d->positions.clear();
	d->normals.clear();
	d->textures.clear();
	d->materials.clear();
}

bool GMModelReader_Obj::load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModels** models)
{
	D(d);
	init();

	buffer.convertToStringBuffer();
	GMString text((char*)buffer.buffer);
	StringReader<GMwchar> sr(text.c_str());
	d->models = new GMModels();

	// 事先分配一些内存，提高效率
	d->positions.reserve(RESERVED);
	d->textures.reserve(RESERVED);
	d->normals.reserve(RESERVED);

	GMString line;
	line.reserve(LINE_MAX);
	while (sr.readLine(line))
	{
		GMScanner s(line);
		GMString token;
		s.next(token);

		if (token == L"#")
		{
			// comment
		}
		else if (token == L"v")
		{
			// vertex
			pushBackData<3>(s, d->positions);
		}
		else if (token == L"vn")
		{
			// normal
			pushBackData<3>(s, d->normals);
		}
		else if (token == L"vt")
		{
			// texture
			pushBackData<2>(s, d->textures);
		}
		else if (token == L"mtllib")
		{
			// material
			GMString name;
			s.next(name);
			loadMaterial(settings, name);
		}
		else if (token == L"usemtl")
		{
			// use material
			GMString name;
			s.next(name);
			d->currentMaterialName = name;
			d->currentModel = nullptr;
		}
		else if (token == L"f")
		{
			// face
			appendFace(s);
		}
	}

	*models = d->models;
	return true;
}

bool GMModelReader_Obj::test(const GMBuffer& buffer)
{
	return buffer.buffer && buffer.buffer[0] == '#';
}

void GMModelReader_Obj::appendFace(GMScanner& scanner)
{
	D(d);
	const ModelReader_Obj_Material& material = d->materials[d->currentMaterialName];

	if (!d->currentModel)
	{
		d->currentModel = new GMModel();
		d->models->push_back(d->currentModel);
		new GMMesh(d->currentModel);

		d->currentModel->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
		applyMaterial(material, d->currentModel->getShader());
	}

	GMint verticesCount = 0;
	GMFloat4 firstVertex, firstNormal, firstTexcoord;
	GMFloat4 lastVertex, lastNormal, lastTexcoord;
	GMString face;
	GMMesh* currentMesh = d->currentModel->getMeshes().front();
	GM_ASSERT(currentMesh);
	while (true)
	{
		scanner.next(face);
		if (face.isEmpty())
			break;

		GMScanner faceScanner(face, false, slashPredicate);
		GMFloat4 f4_vec;

		GMint v = INVALID, t = INVALID, n = INVALID;
		faceScanner.nextInt(&v);
		faceScanner.nextInt(&t);
		faceScanner.nextInt(&n);

		GM_ASSERT(v != INVALID);
		{
			if (verticesCount >= 3)
			{
				// 如果大于三个顶点组成的多边形，需要补入第一个顶点和上一个顶点，形成闭环，下同
				currentMesh->vertex(firstVertex[0], firstVertex[1], firstVertex[2]);
				currentMesh->vertex(lastVertex[0], lastVertex[1], lastVertex[2]);
			}

			auto& vec = d->positions[v - 1];
			vec.loadFloat4(f4_vec);
			currentMesh->vertex(f4_vec[0], f4_vec[1], f4_vec[2]);

			if (!verticesCount)
				firstVertex = f4_vec;
			lastVertex = f4_vec;
		}

		{
			if (verticesCount >= 3)
			{
				currentMesh->texcoord(firstTexcoord[0], firstTexcoord[1]);
				currentMesh->texcoord(lastTexcoord[0], lastTexcoord[1]);
			}

			auto&& vec = t != INVALID ? d->textures[t - 1] : GMVec2(0, 0);
			vec.loadFloat4(f4_vec);
			currentMesh->texcoord(f4_vec[0], f4_vec[1]);

			if (!verticesCount)
				firstTexcoord = f4_vec;
			lastTexcoord = f4_vec;
		}

		GM_ASSERT(n != INVALID);
		{
			if (verticesCount >= 3)
			{
				currentMesh->normal(firstNormal[0], firstNormal[1], firstNormal[2]);
				currentMesh->normal(lastNormal[0], lastNormal[1], lastNormal[2]);
			}

			auto& vec = d->normals[n - 1];
			vec.loadFloat4(f4_vec);
			currentMesh->normal(f4_vec[0], f4_vec[1], f4_vec[2]);

			if (!verticesCount)
				firstNormal = f4_vec;
			lastNormal = f4_vec;
		}

		++verticesCount;
	};
	// 这里其实有优化的余地
	// 如果恰好是4个顶点，可以不采用Triangles(Triangle List)的拓扑，而是使用Triangle Stripe，将第一个顶点插入。
}

void GMModelReader_Obj::loadMaterial(const GMModelLoadSettings& settings, const GMString& mtlFilename)
{
	D(d);
	GMString fn = settings.directory;
	if (fn[fn.length() - 1] != '\\' &&
		fn[fn.length() - 1] != '/')
	{
		fn.append("/");
	}
	fn.append(mtlFilename);
	GMBuffer buffer;

	if (settings.type == GMModelPathType::Relative)
		GM.getGamePackageManager()->readFile(GMPackageIndex::Models, fn, &buffer);
	else
		GM.getGamePackageManager()->readFileFromPath(fn, &buffer);

	buffer.convertToStringBuffer();

	GMString text((char*)buffer.buffer);
	StringReader<GMwchar> sr(text.c_str());
	ModelReader_Obj_Material* material = nullptr;

	GMString line;
	line.reserve(LINE_MAX);
	while (sr.readLine(line))
	{
		GMScanner s(line);
		GMString token;
		s.next(token);
		if (token == L"newmtl")
		{
			GMString name;
			s.next(name);
			material = &(d->materials[name]);
			memset(material, 0, sizeof(*material));
		}
		else if (token == L"Ns")
		{
			GM_ASSERT(material);
			s.nextFloat(&material->ns);
		}
		else if (token == L"Kd")
		{
			GM_ASSERT(material);
			s.nextFloat(&material->kd[0]);
			s.nextFloat(&material->kd[1]);
			s.nextFloat(&material->kd[2]);
		}
		else if (token == L"Ka")
		{
			GM_ASSERT(material);
			s.nextFloat(&material->ka[0]);
			s.nextFloat(&material->ka[1]);
			s.nextFloat(&material->ka[2]);
		}
		else if (token == L"Ks")
		{
			GM_ASSERT(material);
			s.nextFloat(&material->ks[0]);
			s.nextFloat(&material->ks[1]);
			s.nextFloat(&material->ks[2]);
		}
	}
}

void GMModelReader_Obj::applyMaterial(const ModelReader_Obj_Material& material, GMShader& shader)
{
	shader.setCull(GMS_Cull::NONE);

	GMMaterial& m = shader.getMaterial();
	m.ka = MakeVector3(material.ka);
	m.kd = MakeVector3(material.kd);
	m.ks = MakeVector3(material.ks);
	m.shininess = material.ns;
}
