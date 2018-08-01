#include "stdafx.h"
#include "gmmodelreader_md5anim.h"

// Handlers
BEGIN_DECLARE_MD5_HANDLER(MD5Version, reader, scanner, GMModelReader_MD5Anim*)
	GMint version;
	scanner.nextInt(version);
	reader->setMD5Version(version);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(commandline, reader, scanner, GMModelReader_MD5Anim*)
	GMString content;
	scanner.next(content);
	reader->setCommandline(content);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(numFrames, reader, scanner, GMModelReader_MD5Anim*)
	GMint i;
	scanner.nextInt(i);
	reader->setNumFrames(i);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(numJoints, reader, scanner, GMModelReader_MD5Anim*)
	GMint i;
	scanner.nextInt(i);
	reader->setNumJoints(i);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(frameRate, reader, scanner, GMModelReader_MD5Anim*)
	GMint i;
	scanner.nextInt(i);
	reader->setFrameRate(i);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(numAnimatedComponents, reader, scanner, GMModelReader_MD5Anim*)
	GMint i;
	scanner.nextInt(i);
	reader->setNumAnimatedComponents(i);
	return true;
END_DECLARE_MD5_HANDLER()

/*
hierarchy {
<string:jointName> <int:parentIndex> <int:flags> <int:startIndex>
...
}*/
BEGIN_DECLARE_MD5_HANDLER(hierarchy, reader, scanner, GMModelReader_MD5Anim*)
	GMString content;
	scanner.next(content);
	if (content != L"{")
		return false;
	IMd5MeshHandler* nextHandler = reader->findHandler(L"hierarchy_inner");
	GM_ASSERT(nextHandler);
	reader->setNextHandler(nextHandler);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(hierarchy_inner, reader, scanner, GMModelReader_MD5Anim*)
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
		// 这里有问题
		// <string:jointName> <int:parentIndex> <int:flags> <int:startIndex> //
		GMModelReader_MD5Anim_Hierarchy hierarchy;
		scanner.next(hierarchy.jointName);
		scanner.nextInt(hierarchy.parentIndex);
		scanner.nextInt(hierarchy.flags);
		scanner.nextInt(hierarchy.startIndex);
		reader->setHierarchy(std::move(hierarchy));
	}
	return true;
END_DECLARE_MD5_HANDLER()
//////////////////////////////////////////////////////////////////////////
bool GMModelReader_MD5Anim::load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModels** models)
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

bool GMModelReader_MD5Anim::test(const GMBuffer& buffer)
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

Vector<GMOwnedPtr<IMd5MeshHandler>>& GMModelReader_MD5Anim::getHandlers()
{
	D(d);
	if (d->handlers.empty())
	{
		d->handlers.push_back(NEW_MD5_HANDLER(MD5Version));
		d->handlers.push_back(NEW_MD5_HANDLER(commandline));
		d->handlers.push_back(NEW_MD5_HANDLER(numFrames));
		d->handlers.push_back(NEW_MD5_HANDLER(numJoints));
		d->handlers.push_back(NEW_MD5_HANDLER(frameRate));
		d->handlers.push_back(NEW_MD5_HANDLER(numAnimatedComponents));
		d->handlers.push_back(NEW_MD5_HANDLER(hierarchy));
		d->handlers.push_back(NEW_MD5_HANDLER(hierarchy_inner));
	}
	return d->handlers;
}