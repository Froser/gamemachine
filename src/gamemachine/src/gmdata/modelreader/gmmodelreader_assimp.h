#ifndef __GMMODELREADER_ASSIMP_H__
#define __GMMODELREADER_ASSIMP_H__
#include <gmcommon.h>
#include <gmmodelreader.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMModelReader_Assimp)
{
	HashMap<GMString, GMAsset, GMStringHashFunctor> textureMap;
	GMModelLoadSettings settings;
};

class GMModelReader_Assimp : public GMObject, public IModelReader
{
	GM_DECLARE_PRIVATE(GMModelReader_Assimp)

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset) override;
	virtual bool test(const GMModelLoadSettings& settings, const GMBuffer& buffer) override;

public:
	inline HashMap<GMString, GMAsset, GMStringHashFunctor>& getTextureMap() GM_NOEXCEPT
	{
		D(d);
		return d->textureMap;
	}

	inline const GMModelLoadSettings& getSettings()
	{
		D(d);
		return d->settings;
	}
};

END_NS
#endif