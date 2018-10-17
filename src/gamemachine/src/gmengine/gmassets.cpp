#include "stdafx.h"
#include "gmassets.h"
#include "gmdata/gmmodel.h"
#include "gmphysics/gmphysicsshape.h"

GM_DEFINE_ASSET_GETTER(ITexture*, getTexture, GMAssetType::Texture);
GM_DEFINE_ASSET_GETTER(GMModel*, getModel, GMAssetType::Model);
GM_DEFINE_ASSET_GETTER(GMModels*, getModels, GMAssetType::Models);
GM_DEFINE_ASSET_GETTER(GMPhysicsShape*, getPhysicsShape, GMAssetType::PhysicsShape);

GMAsset::GMAsset()
{
	D(d);
	d->ref = new GMAtomic<GMlong>(1);
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
	D(d);
	release();
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
	D(d);
	D_OF(d_asset, &asset);
	GM_ASSERT(d_asset->ref);

	if (this != &asset)
		release();

	d->type = d_asset->type;
	d->asset = d_asset->asset;
	d->ref = d_asset->ref;
	addRef();

	return *this;
}

GMAsset& GMAsset::operator=(GMAsset&& asset) GM_NOEXCEPT
{
	if (this != &asset)
		m_data.swap(asset.m_data);
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
	case GMAssetType::Models:
		GM_delete(getModels());
		break;
	case GMAssetType::PhysicsShape:
		GM_delete(getPhysicsShape());
		break;
	default:
		GM_ASSERT(false);
		break;
	}
	d->asset = nullptr;
	GM_delete(d->ref);
}

const GMAsset& GMAsset::invalidAsset()
{
	static GMAsset s_invalid;
	return s_invalid;
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