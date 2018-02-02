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
	d->model = nullptr;
	d->currentComponent = nullptr;
	d->positions.clear();
	d->normals.clear();
	d->textures.clear();
	d->materials.clear();
}

bool GMModelReader_Obj::load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModel** object)
{
	D(d);
	init();

	buffer.convertToStringBuffer();
	GMString text((char*)buffer.buffer);
	StringReader<GMwchar> sr(text.c_str());
	d->model = new GMModel();

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
			d->currentComponent = nullptr;
		}
		else if (token == L"f")
		{
			// face
			appendFace(s);
		}
	}

	*object = d->model;
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

	GMMesh* mesh = d->model->getMesh();
	if (!d->currentComponent)
	{
		d->currentComponent = new GMComponent(mesh);
		applyMaterial(material, d->currentComponent->getShader());
	}

	GMString face;
	d->currentComponent->beginFace();
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
			auto& vec = d->positions[v - 1];
			vec.loadFloat4(f4_vec);
			d->currentComponent->vertex(f4_vec[0], f4_vec[1], f4_vec[2]);
		}

		{
			auto&& vec = t != INVALID ? d->textures[t - 1] : GMVec2(0, 0);
			vec.loadFloat4(f4_vec);
			d->currentComponent->uv(f4_vec[0], f4_vec[1]);
		}

		GM_ASSERT(n != INVALID);
		{
			auto& vec = d->normals[n - 1];
			vec.loadFloat4(f4_vec);
			d->currentComponent->normal(f4_vec[0], f4_vec[1], f4_vec[2]);
		}
	};
	d->currentComponent->endFace();
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
