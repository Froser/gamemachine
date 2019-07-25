#ifndef __GMMODELREADER_ASSIMP_H__
#define __GMMODELREADER_ASSIMP_H__
#include <gmcommon.h>
#include <gmmodelreader.h>
BEGIN_NS

GM_PRIVATE_CLASS(GMModelReader_Assimp);
class GMModelReader_Assimp : public IModelReader
{
	GM_DECLARE_PRIVATE(GMModelReader_Assimp)

public:
	GMModelReader_Assimp();
	~GMModelReader_Assimp();

	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset) override;
	virtual bool test(const GMModelLoadSettings& settings, const GMBuffer& buffer) override;

public:
	const GMModelLoadSettings& getSettings();
	HashMap<GMString, GMAsset, GMStringHashFunctor>& getTextureMap() GM_NOEXCEPT;
};

END_NS
#endif