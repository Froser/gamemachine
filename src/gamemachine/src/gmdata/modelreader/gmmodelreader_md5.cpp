#include "stdafx.h"
#include "gmmodelreader_md5.h"
#include "gmmodelreader_md5mesh.h"
#include "foundation/gamemachine.h"
#include "gmmodelreader_md5anim.h"

namespace
{
	GMString removeQuotes(const GMString& string)
	{
		bool bLQ = !string.isEmpty() && string[0] == '"';
		bool bRQ = !string.isEmpty() && string[string.length() - 1] == '"';
		return string.substr(bLQ ? 1 : 0, bRQ ? string.length() - 2 : string.length() - 1);
	}
}

BEGIN_DECLARE_MD5_HANDLER(GMMD5Version, reader, scanner, GMModelReader_MD5*)
	GMint version;
	scanner.nextInt(version);
	reader->setMD5Version(version);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(md5mesh, reader, scanner, GMModelReader_MD5*)
	GMString content;
	scanner.next(content);
	reader->setMeshFile(removeQuotes(content));
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(md5anim, reader, scanner, GMModelReader_MD5*)
	GMString content;
	scanner.next(content);
	reader->setAnimFile(removeQuotes(content));
	return true;
END_DECLARE_MD5_HANDLER()

GMVec3 GMMD5VectorParser::parseVector3(GMScanner& s)
{
	// 解析(x,y,z)，并返回一个三维向量
	static GMVec3 s_defaultVec(0, 0, 0);
	GMVec3 result;
	GMString symbol;
	s.next(symbol);
	if (symbol != L"(")
	{
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Wrong vector format."));
		return s_defaultVec;
	}

	GMfloat x, y, z;
	if (s.nextFloat(x) && s.nextFloat(y) && s.nextFloat(z))
	{
		result = GMVec3(x, y, z);
	}
	else
	{
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Wrong vector format."));
		return s_defaultVec;
	}

	s.next(symbol);
	if (symbol != L")")
	{
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Wrong vector format."));
		return s_defaultVec;
	}

	return result;
}

GMVec2 GMMD5VectorParser::parseVector2(GMScanner& s)
{
	// 解析(x,y)，并返回一个二维向量
	static GMVec2 s_defaultVec(0, 0);
	GMVec2 result;
	GMString symbol;
	s.next(symbol);
	if (symbol != L"(")
	{
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Wrong vector format."));
		return s_defaultVec;
	}

	GMfloat x, y;
	if (s.nextFloat(x) && s.nextFloat(y))
	{
		result = GMVec2(x, y);
	}
	else
	{
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Wrong vector format."));
		return s_defaultVec;
	}

	s.next(symbol);
	if (symbol != L")")
	{
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Wrong vector format."));
		return s_defaultVec;
	}

	return result;
}

GMQuat GMMD5VectorParser::parseQuatFromVector3(GMScanner& s)
{
	GMVec3 quat3 = parseVector3(s);
	// 根据四元数模为1，计算出四元数的w
	GMfloat t = 1.0f - (quat3.getX() * quat3.getX()) - (quat3.getY() * quat3.getY()) - (quat3.getZ() * quat3.getZ());
	GMfloat w;
	if (t < 0.0f)
	{
		w = 0.0f;
	}
	else
	{
		w = -Sqrt(t);
	}

	return GMQuat(quat3.getX(), quat3.getY(), quat3.getZ(), w);
}

bool GMModelReader_MD5::load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModels** models)
{
	D(d);
	static GMModelReader_MD5Mesh s_meshReader;
	static GMModelReader_MD5Anim s_animReader;

	buffer.convertToStringBuffer();
	GMString text = GMString(reinterpret_cast<char*>(buffer.buffer));
	StringReader<GMwchar> sr(text.c_str());

	GMString line;
	GMString tag, content;
	line.reserve(10);
	while (sr.readLine(line))
	{
		GMScanner s(line);
		s.next(tag);
		IMd5MeshHandler* handler = findHandler(tag);
		if (handler)
		{
			if (!handler->handle(this, s))
				return false;
		}
	}

	// 已经获得了mesh和anim文件，开始解析
	{
		GMString filename = settings.directory + "/" + d->meshFile;
		GMBuffer meshBuffer;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Models, filename, &meshBuffer);
		GM_ASSERT(s_meshReader.test(meshBuffer));
		s_meshReader.load(GMModelLoadSettings(filename, settings.directory), meshBuffer, nullptr);
	}

	{
		GMString filename = settings.directory + "/" + d->animFile;
		GMBuffer meshBuffer;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Models, filename, &meshBuffer);
		GM_ASSERT(s_animReader.test(meshBuffer));
		s_animReader.load(GMModelLoadSettings(filename, settings.directory), meshBuffer, nullptr);
	}

	return true;
}

bool GMModelReader_MD5::test(const GMBuffer& buffer)
{
	// 使用GameMachine的一种MD5格式，其实里面就是索引了mesh和anim文件
	if (buffer.size > 14)
	{
		char head[15] = { 0 };
		GMString::stringCopyN(head, 15, reinterpret_cast<const char*>(buffer.buffer), 14);
		if (GMString::stringEquals(head, "GMMD5Version 0"))
			return true;
	}

	return false;
}

Vector<GMOwnedPtr<IMd5MeshHandler>>& GMModelReader_MD5::getHandlers()
{
	D(d);
	if (d->handlers.empty())
	{
		d->handlers.push_back(NEW_MD5_HANDLER(GMMD5Version));
		d->handlers.push_back(NEW_MD5_HANDLER(md5mesh));
		d->handlers.push_back(NEW_MD5_HANDLER(md5anim));
	}
	return d->handlers;
}

IMd5MeshHandler* GMModelReader_MD5::findHandler(const GMString& tag)
{
	auto& handlers = getHandlers();
	for (decltype(auto) handler : handlers)
	{
		if (handler->canHandle(tag))
			return handler.get();
	}
	return nullptr;
}