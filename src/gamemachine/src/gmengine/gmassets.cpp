#include "stdafx.h"
#include "gmassets.h"
#include "gmdata/gmmodel.h"
#include "gmphysics/gmphysicsshape.h"

BEGIN_NS
static GMAsset s_invalid;

GM_DEFINE_ASSET_GETTER(ITexture*, Texture, GMAssetType::Texture);
GM_DEFINE_ASSET_GETTER(GMModel*, Model, GMAssetType::Model);
GM_DEFINE_ASSET_GETTER(GMScene*, Scene, GMAssetType::Scene);
GM_DEFINE_ASSET_GETTER(GMPhysicsShape*, PhysicsShape, GMAssetType::PhysicsShape);

GM_PRIVATE_OBJECT_UNALIGNED(GMAsset)
{
	GMSharedPtr<GMAtomic<GMlong>> ref;
	GMAssetType type = GMAssetType::Invalid;
	void* asset = nullptr;
};

GMAsset::GMAsset()
{
	GM_CREATE_DATA();

	D(d);
	d->ref = std::make_shared<GMAtomic<GMlong>>(1);
}

GMAsset::GMAsset(GMAssetType type, void* asset)
	: GMAsset()
{
	D(d);
	d->type = type;
	d->asset = asset;
}

GMAsset::~GMAsset()
{
	release();
}

GMAssetType GMAsset::getType() const GM_NOEXCEPT
{
	D(d);
	return d->type;
}

void GMAsset::setType(GMAssetType type) GM_NOEXCEPT
{
	D(d);
	d->type = type;
}

bool GMAsset::isEmpty() const GM_NOEXCEPT
{
	D(d);
	return !d->asset;
}

void* GMAsset::getAsset() const GM_NOEXCEPT
{
	D(d);
	return d->asset;
}

GMAsset::GMAsset(const GMAsset& asset)
{
	*this = asset;
}

GMAsset::GMAsset(GMAsset&& asset) GM_NOEXCEPT
{
	*this = std::move(asset);
}

GMAsset& GMAsset::operator=(const GMAsset& asset)
{
	if (this != &asset)
		release();

	GM_COPY(asset);
	addRef();

	return *this;
}

GMAsset& GMAsset::operator=(GMAsset&& asset) GM_NOEXCEPT
{
	GM_MOVE(asset);
	return *this;
}

void GMAsset::addRef()
{
	D(d);
	if (isEmpty())
		return;

	if (d->ref)
		++(*d->ref);
}

void GMAsset::release()
{
	if (!_gm_data)
		return;

	D(d);
	if (isEmpty())
		return;

	if (d->ref)
	{
		(*d->ref)--;
		if ((*d->ref) == 0)
			removeData();
	}
}

void GMAsset::removeData()
{
	D(d);
	switch (getType())
	{
	case GMAssetType::Invalid:
		break;
	case GMAssetType::Texture:
		GM_delete(getTexture());
		break;
	case GMAssetType::Model:
		GM_delete(getModel());
		break;
	case GMAssetType::Scene:
		GM_delete(getScene());
		break;
	case GMAssetType::PhysicsShape:
		GM_delete(getPhysicsShape());
		break;
	default:
		GM_ASSERT(false);
		break;
	}
	d->asset = nullptr;
}

const GMAsset& GMAsset::invalidAsset()
{
	return s_invalid;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMAssets)
{
	Vector<GMAsset> unnamedAssets;
	HashMap<GMString, GMAsset, GMStringHashFunctor> childs;
};

GMAssets::GMAssets()
{
	GM_CREATE_DATA();
}

GMAssets::~GMAssets()
{

}

GMAsset GMAssets::addAsset(GMAsset asset)
{
	D(d);
	d->unnamedAssets.push_back(asset);
	return asset;
}

GMAsset GMAssets::addAsset(const GMString& name, GMAsset asset)
{
	D(d);
	auto result = d->childs.insert({ name, asset });
	if (result.second)
		return asset;
	return GMAsset::invalidAsset();
}

GMAsset GMAssets::getAsset(GMsize_t index)
{
	D(d);
	return d->unnamedAssets[index];
}

GMAsset GMAssets::getAsset(const GMString& name)
{
	D(d);
	auto iter = d->childs.find(name);
	if (iter != d->childs.end())
		return iter->second;
	return GMAsset::invalidAsset();
}
END_NS