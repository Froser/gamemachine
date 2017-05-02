#include "stdafx.h"
#include "modelreader.h"
#include "utilities/assert.h"
#include "modelreader_obj.h"
#include "gmdatacore/gamepackage.h"

class ModelReaderContainer
{
public:
	ModelReaderContainer()
	{
		m_readers[ModelType_Obj] = new ModelReader_Obj();
	}

	~ModelReaderContainer()
	{
		for (auto iter = m_readers.begin(); iter != m_readers.end(); iter++)
		{
			delete iter->second;
		}
	}

	IModelReader* getReader(ModelType type)
	{
		ASSERT(m_readers.find(type) != m_readers.end());
		return m_readers[type];
	}

private:
	std::map<ModelType, IModelReader*> m_readers;
};

IModelReader* ModelReader::getReader(ModelType type)
{
	static ModelReaderContainer readers;
	return readers.getReader(type);
}

ModelType ModelReader::test(const GamePackageBuffer& buffer)
{
	for (ModelType i = ModelType_Begin; i < ModelType_End; i = (ModelType)((GMuint)i + 1))
	{
		if (getReader(i)->test(buffer))
			return i;
	}
	return ModelType_End;
}


bool ModelReader::load(const ModelLoadSettings& settings, OUT Object** image)
{
	return load(settings, ModelType_AUTO, image);
}

bool ModelReader::load(const ModelLoadSettings& settings, ModelType type, OUT Object** object)
{
	GamePackageBuffer buffer;
	settings.gamePackage.readFileFromPath(settings.path, &buffer);

	if (type == ModelType_AUTO)
		type = test(buffer);

	if (type == ModelType_End)
		return false;

	return getReader(type)->load(settings, buffer, object);
}
