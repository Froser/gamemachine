#include "stdafx.h"
#include "gmmodelreader.h"
#include "gmmodelreader_obj.h"
#include "gmdatacore/gamepackage.h"

GM_ALIGNED_16(class) ModelReaderContainer : public GMObject
{
public:
	ModelReaderContainer()
	{
		m_readers[GMModelReader::ModelType_Obj] = new GMModelReader_Obj();
	}

	~ModelReaderContainer()
	{
		for (auto iter = m_readers.begin(); iter != m_readers.end(); iter++)
		{
			delete iter->second;
		}
	}

	IModelReader* getReader(GMModelReader::ModelType type)
	{
		ASSERT(m_readers.find(type) != m_readers.end());
		return m_readers[type];
	}

private:
	std::map<GMModelReader::ModelType, IModelReader*> m_readers;
};

IModelReader* GMModelReader::getReader(ModelType type)
{
	static ModelReaderContainer readers;
	return readers.getReader(type);
}

GMModelReader::ModelType GMModelReader::test(const GamePackageBuffer& buffer)
{
	for (ModelType i = ModelType_Begin; i < ModelType_End; i = (ModelType)((GMuint)i + 1))
	{
		if (getReader(i)->test(buffer))
			return i;
	}
	return ModelType_End;
}


bool GMModelReader::load(const GMModelLoadSettings& settings, OUT Object** object)
{
	return load(settings, ModelType_AUTO, object);
}

bool GMModelReader::load(const GMModelLoadSettings& settings, ModelType type, OUT Object** object)
{
	GamePackageBuffer buffer;
	settings.gamePackage.readFileFromPath(settings.path, &buffer);

	if (type == ModelType_AUTO)
		type = test(buffer);

	if (type == ModelType_End)
		return false;

	return getReader(type)->load(settings, buffer, object);
}
