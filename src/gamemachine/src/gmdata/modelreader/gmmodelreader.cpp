#include "stdafx.h"
#include "gmmodelreader.h"
// #include "gmmodelreader_md5.h"
#include "gmmodelreader_assimp.h"
#include "gmdata/gamepackage/gmgamepackage.h"
#include "foundation/gamemachine.h"

class GMModelReaderContainer : public GMObject
{
public:
	GMModelReaderContainer()
	{
		// m_readers[GMModelReader::GMMd5] = new GMModelReader_MD5();
		m_readers[GMModelReader::Assimp] = new GMModelReader_Assimp();
	}

	~GMModelReaderContainer()
	{
		for (auto iter = m_readers.begin(); iter != m_readers.end(); iter++)
		{
			GM_delete(iter->second);
		}
	}

	IModelReader* getReader(GMModelReader::EngineType type)
	{
		GM_ASSERT(m_readers.find(type) != m_readers.end());
		return m_readers[type];
	}

private:
	Map<GMModelReader::EngineType, IModelReader*> m_readers;
};

IModelReader* GMModelReader::getReader(EngineType type)
{
	static GMModelReaderContainer readers;
	return readers.getReader(type);
}

GMModelReader::EngineType GMModelReader::test(const GMModelLoadSettings& settings, const GMBuffer& buffer)
{
	for (EngineType i = ModelType_Begin; i < ModelType_End; i = (EngineType)((GMuint32)i + 1))
	{
		if (getReader(i)->test(settings, buffer))
			return i;
	}
	return ModelType_End;
}

bool GMModelReader::load(const GMModelLoadSettings& settings, REF GMSceneAsset& asset)
{
	return load(settings, Auto, asset);
}

bool GMModelReader::load(const GMModelLoadSettings& settings, EngineType type, REF GMSceneAsset& asset)
{
	GMModelLoadSettings settingsCache = settings;
	settingsCache.directory = settings.directory.isEmpty() ? GMPath::directoryName(settings.filename) : settings.directory;

	GMBuffer buffer;
	if (settingsCache.type == GMModelPathType::Relative)
		GM.getGamePackageManager()->readFile(GMPackageIndex::Models, settingsCache.filename, &buffer);
	else
		GM.getGamePackageManager()->readFileFromPath(settingsCache.filename, &buffer);

	if (type == Auto)
		type = test(settingsCache, buffer);

	if (type == ModelType_End)
		return false;

	return getReader(type)->load(settingsCache, buffer, asset);
}
