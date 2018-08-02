#include "stdafx.h"
#include "gmmodelreader_md5mesh.h"
#include "foundation/utilities/tools.h"

// Handlers
BEGIN_DECLARE_MD5_HANDLER(MD5Version, reader, scanner, GMModelReader_MD5Mesh*)
	GMint version;
	scanner.nextInt(version);
	reader->setMD5Version(version);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(commandline, reader, scanner, GMModelReader_MD5Mesh*)
	GMString content;
	scanner.next(content);
	reader->setCommandline(content);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(numJoints, reader, scanner, GMModelReader_MD5Mesh*)
	GMint numJoints;
	scanner.nextInt(numJoints);
	reader->setNumJoints(numJoints);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(numMeshes, reader, scanner, GMModelReader_MD5Mesh*)
	GMint numMeshes;
	scanner.nextInt(numMeshes);
	reader->setNumMeshes(numMeshes);
	return true;
END_DECLARE_MD5_HANDLER()

/*
joints {
<string:name> <int:parentIndex> ( <vec3:position> ) ( <vec3:orientation> )
...
}
*/
BEGIN_DECLARE_MD5_HANDLER(joints, reader, scanner, GMModelReader_MD5Mesh*)
	GMString content;
	scanner.next(content);
	if (content != L"{")
		return false;
	IMd5MeshHandler* nextHandler = reader->findHandler(L"joints_inner");
	GM_ASSERT(nextHandler);
	reader->setNextHandler(nextHandler);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(joints_inner, reader, scanner, GMModelReader_MD5Mesh*)
	GMString content;
	scanner.next(content);
	if (content == L"}")
	{
		reader->setNextHandler(nullptr);
		return true;
	}
	else if (content == L"")
	{
		// 不处理空行
		return true;
	}
	else
	{
		// <string:name> <int:parentIndex> ( <vec3:position> ) ( <vec3:orientation> )
		GMModelReader_MD5Mesh_Joint joint;
		joint.name = content;
		scanner.nextInt(joint.parentIndex);
		joint.position = GMMD5VectorParser::parseVector3(scanner);
		joint.orientation = GMMD5VectorParser::parseVector3(scanner);

		scanner.next(content);
		if (content == "//")
		{
			scanner.next(joint.annotation);
		}

		reader->addJoint(std::move(joint));
	}
	return true;
END_DECLARE_MD5_HANDLER()

/*
mesh {
shader <string:texture>
numverts <int:numVerts>
vert <int:vertexIndex> ( <vec2:texCoords> ) <int:startWeight> <int:weightCount>
numtris <int:numTriangles>
tri <int:triangleIndex> <int:vertIndex0> <int:vertIndex1> <int:vertIndex2>
numweights <int:numWeights>
weight <int:weightIndex> <int:jointIndex> <float:weightBias> ( <vec3:weightPosition> )
...
}
*/
struct Handler_mesh_inner : IMd5MeshHandler
{
	virtual bool canHandle(const GMString& tag) override
	{
		return tag == L"mesh_inner";
	}

	virtual bool handle(GMModelReader_MD5* r, GMScanner& scanner) override
	{
		GMModelReader_MD5Mesh* reader = gm_cast<GMModelReader_MD5Mesh*>(r);
		GMString content;
		scanner.next(content);

		if (content == L"shader")
		{
			scanner.next(content);
			m_cacheMesh->shader = content;
		}
		else if (content == L"numverts")
		{
			GMint n;
			scanner.nextInt(n);
			m_cacheMesh->numVertices = n;
		}
		else if (content == L"")
		{
			// 不处理空行
			return true;
		}
		else if (content == L"}")
		{
			if (m_cacheMesh)
				reader->addMesh(std::move(*m_cacheMesh));
			reader->setNextHandler(nullptr);
			return true;
		}
		else if (content == L"vert")
		{
			// vert <int:vertexIndex> ( <vec2:texCoords> ) <int:startWeight> <int:weightCount>
			GMint i;
			GMModelReader_MD5Mesh_Vertex v;
			scanner.nextInt(i);
			v.vertexIndex = i;
			v.texCoords = GMMD5VectorParser::parseVector2(scanner);
			scanner.nextInt(i);
			v.startWeight = i;
			scanner.nextInt(i);
			v.weightCount = i;
			m_cacheMesh->vertices.push_back(std::move(v));
		}
		else if (content == L"numtris")
		{
			GMint n;
			scanner.nextInt(n);
			m_cacheMesh->numTriangles = n;
		}
		else if (content == L"tri")
		{
			// tri <int:triangleIndex> <int:vertIndex0> <int:vertIndex1> <int:vertIndex2>
			GMint indices[4];
			scanner.nextInt(indices[0]);
			scanner.nextInt(indices[1]);
			scanner.nextInt(indices[2]);
			scanner.nextInt(indices[3]);
			m_cacheMesh->triangleIndices.push_back(GMVec4(indices[0], indices[1], indices[2], indices[3]));
		}
		else if (content == L"numweights")
		{
			GMint n;
			scanner.nextInt(n);
			m_cacheMesh->numWeights = n;
		}
		else if (content == L"weight")
		{
			// weight <int:weightIndex> <int:jointIndex> <float:weightBias> ( <vec3:weightPosition> )
			GMModelReader_MD5Mesh_Weight weight;
			GMint i;
			GMfloat f;
			scanner.nextInt(i);
			weight.weightIndex = i;
			scanner.nextInt(i);
			weight.jointIndex = i;
			scanner.nextFloat(f);
			weight.weightBias = f;
			weight.weightPosition = GMMD5VectorParser::parseVector3(scanner);
			m_cacheMesh->weights.push_back(std::move(weight));
		}

		return true;
	}

	void reset()
	{
		m_cacheMesh.reset(new GMModelReader_MD5Mesh_Mesh());
	}

private:
	GMOwnedPtr<GMModelReader_MD5Mesh_Mesh> m_cacheMesh;
};

BEGIN_DECLARE_MD5_HANDLER(mesh, reader, scanner, GMModelReader_MD5Mesh*)
	GMString content;
	scanner.next(content);
	if (content != L"{")
		return false;
	Handler_mesh_inner* nextHandler = gm_cast<Handler_mesh_inner*>(reader->findHandler(L"mesh_inner"));
	GM_ASSERT(nextHandler);
	nextHandler->reset();
	reader->setNextHandler(nextHandler);
	return true;
END_DECLARE_MD5_HANDLER()

//////////////////////////////////////////////////////////////////////////
bool GMModelReader_MD5Mesh::load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModels** models)
{
	D(d);
	buffer.convertToStringBuffer();
	GMString text = GMString(reinterpret_cast<char*>(buffer.buffer));
	StringReader<GMwchar> sr(text.c_str());

	GMString line;
	GMString tag, content;
	line.reserve(LINE_MAX);
	while (sr.readLine(line))
	{
		GMScanner s(line);
		if (!d->nextHandler)
		{
			s.next(tag);
			IMd5MeshHandler* handler = findHandler(tag);
			if (handler)
			{
				if (!handler->handle(this, s))
					return false;
			}
		}
		else
		{
			if (!d->nextHandler->handle(this, s))
				return false;
		}
	}
	return true;
}

bool GMModelReader_MD5Mesh::test(const GMBuffer& buffer)
{
	if (buffer.size > 13)
	{
		char head[14] = { 0 };
		GMString::stringCopyN(head, 14, reinterpret_cast<const char*>(buffer.buffer), 13);
		if (GMString::stringEquals(head, "MD5Version 10"))
			return true;
	}

	return false;
}

Vector<GMOwnedPtr<IMd5MeshHandler>>& GMModelReader_MD5Mesh::getHandlers()
{
	D(d);
	if (d->handlers.empty())
	{
		d->handlers.push_back(NEW_MD5_HANDLER(MD5Version));
		d->handlers.push_back(NEW_MD5_HANDLER(commandline));
		d->handlers.push_back(NEW_MD5_HANDLER(numJoints));
		d->handlers.push_back(NEW_MD5_HANDLER(numMeshes));
		d->handlers.push_back(NEW_MD5_HANDLER(joints));
		d->handlers.push_back(NEW_MD5_HANDLER(joints_inner));
		d->handlers.push_back(NEW_MD5_HANDLER(mesh));
		d->handlers.push_back(NEW_MD5_HANDLER(mesh_inner));
	}
	return d->handlers;
}