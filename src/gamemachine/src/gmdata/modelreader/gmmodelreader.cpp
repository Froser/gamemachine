#include "stdafx.h"
#include "gmmodelreader.h"
#include "gmmodelreader_assimp.h"
#include "gmdata/gamepackage/gmgamepackage.h"
#include "foundation/gamemachine.h"

BEGIN_NS

class GMModelReaderContainer : public GMObject
{
public:
	IModelReader* getReader(GMModelReader::EngineType type)
	{
		switch (type)
		{
		case GMModelReader::Assimp:
			return new GMModelReader_Assimp();
			break;
		default:
			gm_error(gm_dbg_wrap("Reader type is not valid"));
			break;
		}
		return nullptr;
	}
};

IModelReader* GMModelReader::createReader(EngineType type)
{
	static GMModelReaderContainer readers;
	return readers.getReader(type);
}

GMModelReader::EngineType GMModelReader::test(const GMModelLoadSettings& settings, const GMBuffer& buffer)
{
	GM_FOREACH_ENUM_CLASS(i, ModelType_Begin, ModelType_End)
	{
		GMOwnedPtr<IModelReader> reader(createReader(i));
		if (reader->test(settings, buffer))
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

	GMOwnedPtr<IModelReader> reader(createReader(type));
	return reader->load(settingsCache, buffer, asset);
}

END_NS