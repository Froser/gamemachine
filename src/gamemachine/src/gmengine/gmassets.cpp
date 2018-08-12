#include "stdafx.h"
#include "gmassets.h"

namespace
{
	static GMAsset s_invalidAsset;
}

GMAsset::GMAsset()
{
	D(d);
	d->ref = new GMAtomic<GMint>(1);
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

	d->ref = d_asset->ref;
	addRef();

	d->type = d_asset->type;
	d->asset = d_asset->asset;
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
	if (d->ref)
		++(*d->ref);
}

void GMAsset::release()
{
	D(d);
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
	case GMAssetType::None:
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
	return s_invalidAsset;
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
	return s_invalidAsset;
}