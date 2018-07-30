#include "stdafx.h"
#include "gmmodelreader_md5mesh.h"

GMModelReader_MD5Mesh::GMModelReader_MD5Mesh()
{

}

GMModelReader_MD5Mesh::~GMModelReader_MD5Mesh()
{

}

bool GMModelReader_MD5Mesh::load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModels** models)
{
	return false;
}

bool GMModelReader_MD5Mesh::test(const GMBuffer& buffer)
{
	if (buffer.size > 10)
	{
		char head[11] = { 0 };
		GMString::stringCopyN(head, 11, reinterpret_cast<const char*>(buffer.buffer), 10);
		if (GMString::stringEquals(head, "MD5Version"))
			return true;
	}

	return false;
}