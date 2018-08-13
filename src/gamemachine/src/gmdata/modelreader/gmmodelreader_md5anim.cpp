#include "stdafx.h"
#include "gmmodelreader_md5anim.h"

namespace
{
	GMSkeletonJoint baseframeToSkeletonJoint(const GMBaseFrame& baseframe)
	{
		GMSkeletonJoint joint;
		joint.setOrientation(baseframe.orientation);
		joint.setPosition(baseframe.position);
		return joint;
	}
}

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
	reader->initFrames(i);
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
		// <string:jointName> <int:parentIndex> <int:flags> <int:startIndex> //
		GMModelReader_MD5Anim_Joint joint;

		joint.jointName = content;
		scanner.nextInt(joint.parentIndex);
		scanner.nextInt(joint.flags);
		scanner.nextInt(joint.startIndex);
		scanner.next(content);
		if (content == "//")
		{
			scanner.next(joint.annotation);
		}
		reader->addJointToHierarchy(std::move(joint));
	}
	return true;
END_DECLARE_MD5_HANDLER()

/*
bounds {
( vec3:boundMin ) ( vec3:boundMax )
...
}
*/
BEGIN_DECLARE_MD5_HANDLER(bounds, reader, scanner, GMModelReader_MD5Anim*)
	GMString content;
	scanner.next(content);
	if (content != L"{")
		return false;
	IMd5MeshHandler* nextHandler = reader->findHandler(L"bounds_inner");
	GM_ASSERT(nextHandler);
	reader->setNextHandler(nextHandler);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(bounds_inner, reader, scanner, GMModelReader_MD5Anim*)
	GMString content;
	scanner.peek(content);
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
		// ( vec3:boundMin ) ( vec3:boundMax )
		GMModelReader_MD5Anim_Bound bound;
		bound.boundMin = GMMD5VectorParser::parseVector3(scanner);
		bound.boundMax = GMMD5VectorParser::parseVector3(scanner);
		reader->addBound(std::move(bound));
	}
	return true;
END_DECLARE_MD5_HANDLER()

/*
baseframe {
( vec3:position ) ( vec3:orientation )
...
}
*/
BEGIN_DECLARE_MD5_HANDLER(baseframe, reader, scanner, GMModelReader_MD5Anim*)
	GMString content;
	scanner.next(content);
	if (content != L"{")
		return false;
	IMd5MeshHandler* nextHandler = reader->findHandler(L"baseframe_inner");
	GM_ASSERT(nextHandler);
	reader->setNextHandler(nextHandler);
	return true;
END_DECLARE_MD5_HANDLER()

BEGIN_DECLARE_MD5_HANDLER(baseframe_inner, reader, scanner, GMModelReader_MD5Anim*)
	GMString content;
	scanner.peek(content);
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
		// ( vec3:position ) ( vec3:orientation )
		GMModelReader_MD5Anim_Baseframe baseframe;
		baseframe.position = GMMD5VectorParser::parseVector3(scanner);
		baseframe.orientation = GMMD5VectorParser::parseQuatFromVector3(scanner);
		reader->addBaseframe(std::move(baseframe));
	}
	return true;
END_DECLARE_MD5_HANDLER()

/*
frame <int:frameNum> {
<float:frameData> ...
}
...
*/
GM_ALIGNED_STRUCT(Handler_frame_inner), IMd5MeshHandler
{
	virtual bool canHandle(const GMString& t) override
	{
		return t == "frame_inner";
	}

	virtual bool handle(GMModelReader_MD5* _reader, GMScanner& scanner)
	{
		GMModelReader_MD5Anim* reader = gm_cast<GMModelReader_MD5Anim*>(_reader);
		GMString content;
		scanner.peek(content);
		if (content == L"}")
		{
			// 提交
			reader->setNextHandler(nullptr);
			reader->setFrame(m_frameNum, std::move(m_frame));
			m_frameNum = -1;
			m_frame.frameData.clear();
			return true;
		}
		else if (content == L"")
		{
			// 不处理空行
			return true;
		}
		else
		{
			// <float:frameData> ...
			GM_ASSERT(m_frameNum != -1);
			GMfloat data;
			while (scanner.nextFloat(data))
			{
				m_frame.frameData.push_back(data);
			}
		}
		return true;
	}

	void setFrameNum(GMint frameNum) GM_NOEXCEPT
	{
		m_frameNum = frameNum;
	}

	void initFrame(GMint size)
	{
		m_frame.frameData.reserve(size);
	}

private:
	GMint m_frameNum = -1;
	GMModelReader_MD5Anim_Frame m_frame;
};

BEGIN_DECLARE_MD5_HANDLER(frame, reader, scanner, GMModelReader_MD5Anim*)
	GMString content;
	scanner.next(content);

	// <int:frameNum>
	Handler_frame_inner* frame_inner = gm_cast<Handler_frame_inner*>(reader->findHandler(L"frame_inner"));
	GM_ASSERT(frame_inner);
	frame_inner->setFrameNum(GMString::parseInt(content));
	frame_inner->initFrame(reader->getNumAnimatedComponents());

	scanner.next(content);
	if (content == L"{")
	{
		IMd5MeshHandler* nextHandler = reader->findHandler(L"frame_inner");
		GM_ASSERT(nextHandler);
		reader->setNextHandler(nextHandler);
		return true;
	}
	return false;
END_DECLARE_MD5_HANDLER()

//////////////////////////////////////////////////////////////////////////
bool GMModelReader_MD5Anim::load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset)
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

	GM_ASSERT(d->numJoints == d->hierarchy.joints.size());
	GM_ASSERT(d->numJoints == d->baseframes.size());
	GM_ASSERT(d->numFrames == d->frames.size());

	GMModels* targetModels = nullptr;
	if (asset.isEmpty())
	{
		targetModels = new GMModels();
		asset = GMAsset(GMAssetType::Models, targetModels);
	}
	else
	{
		targetModels = asset.getModels();
	}
	buildModel(targetModels);
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
		d->handlers.push_back(NEW_MD5_HANDLER(bounds));
		d->handlers.push_back(NEW_MD5_HANDLER(bounds_inner));
		d->handlers.push_back(NEW_MD5_HANDLER(baseframe));
		d->handlers.push_back(NEW_MD5_HANDLER(baseframe_inner));
		d->handlers.push_back(NEW_MD5_HANDLER(frame));
		d->handlers.push_back(NEW_MD5_HANDLER(frame_inner));
	}
	return d->handlers;
}

void GMModelReader_MD5Anim::addJointToHierarchy(GMModelReader_MD5Anim_Joint&& joint)
{
	D(d);
	d->hierarchy.joints.push_back(std::move(joint));
}

void GMModelReader_MD5Anim::addBound(GMModelReader_MD5Anim_Bound&& bounds)
{
	D(d);
	d->bounds.push_back(std::move(bounds));
}

void GMModelReader_MD5Anim::addBaseframe(GMModelReader_MD5Anim_Baseframe&& baseframe)
{
	D(d);
	d->baseframes.push_back(std::move(baseframe));
}

void GMModelReader_MD5Anim::initFrames(GMint num)
{
	D(d);
	d->frames.resize(num);
}

void GMModelReader_MD5Anim::setFrame(GMint index, GMModelReader_MD5Anim_Frame&& frame)
{
	D(d);
	d->frames[index] = std::move(frame);
}

void GMModelReader_MD5Anim::buildModel(GMModels* models)
{
	if (!models)
		return;

	D(d);
	// 标记(flags)
	enum
	{
		Pos_X = 1 << 0,
		Pos_Y = 1 << 1,
		Pos_Z = 1 << 2,
		Ori_X = 1 << 3,
		Ori_Y = 1 << 4,
		Ori_Z = 1 << 5,
	};

	GMSkeleton* skeleton = nullptr;
	if (!(skeleton = models->getSkeleton()))
	{
		skeleton = new GMSkeleton();
		models->setSkeleton(skeleton);
	}
	skeleton->getAnimatedSkeleton().getJoints().assign(d->numJoints, GMSkeletonJoint());
	skeleton->setFrameRate(d->frameRate);

	for (auto frame : d->frames)
	{
		GMFrameSkeleton frameSkeleton;

		// hierarchy和baseframe是一一对应的
		for (GMsize_t i = 0; i < d->hierarchy.joints.size(); ++i)
		{
			GMsize_t j = 0;
			auto& joint = d->hierarchy.joints[i];
			GMSkeletonJoint animatedJoint = baseframeToSkeletonJoint(d->baseframes[i]);
			animatedJoint.setParentIndex(joint.parentIndex);

			if (joint.flags & Pos_X) // Pos.x
			{
				animatedJoint.getPosition().setX(frame.frameData[joint.startIndex + j++]);
			}
			if (joint.flags & Pos_Y) // Pos.y
			{
				animatedJoint.getPosition().setY(frame.frameData[joint.startIndex + j++]);
			}
			if (joint.flags & Pos_Z) // Pos.x
			{
				animatedJoint.getPosition().setZ(frame.frameData[joint.startIndex + j++]);
			}
			if (joint.flags & Ori_X) // Orient.x
			{
				animatedJoint.getOrientation().setX(frame.frameData[joint.startIndex + j++]);
			}
			if (joint.flags & Ori_Y) // Orient.y
			{
				animatedJoint.getOrientation().setY(frame.frameData[joint.startIndex + j++]);
			}
			if (joint.flags & Ori_Z) // Orient.z
			{
				animatedJoint.getOrientation().setZ(frame.frameData[joint.startIndex + j++]);
			}

			GMVec3 quat3(animatedJoint.getOrientation().getX(), animatedJoint.getOrientation().getY(), animatedJoint.getOrientation().getZ());
			animatedJoint.getOrientation().setW(GMMD5VectorParser::calcQuatWFromVector3(quat3));

			if (joint.parentIndex != GMSkeletonJoint::RootIndex)
			{
				GMSkeletonJoint& parent = frameSkeleton.getJoints()[animatedJoint.getParentIndex()];
				GMVec3 rotatePosition = animatedJoint.getPosition() * parent.getOrientation();
				animatedJoint.setPosition(parent.getPosition() + rotatePosition);
				animatedJoint.setOrientation(Normalize(animatedJoint.getOrientation() * parent.getOrientation()));
			}
			frameSkeleton.getJoints().push_back(std::move(animatedJoint));
		}
		skeleton->getSkeletons().push_back(std::move(frameSkeleton));
	}
	GM_ASSERT(skeleton->getSkeletons().size() == static_cast<GMsize_t>(d->numFrames));
}