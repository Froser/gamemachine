#ifndef __GMUICONFIGURATION_H__
#define __GMUICONFIGURATION_H__
#include <gmcommon.h>
#include <gmwidget.h>
BEGIN_NS

class GMWidgetResourceManager;
class GMWidget;

struct GMUITexture
{
	GMlong id;
	GMint32 width;
	GMint32 height;
	GMTextureAsset asset;
};

struct GMUIArea
{
	GMTextureArea::Area area;
	GMlong textureId;
	GMRect rc;
	GMRect cornerRc;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMUIConfiguration)
{
	const IRenderContext* context;
	Vector<GMUITexture> textures;
	Vector<GMUIArea> areas;
	Map<GMlong, GMlong> textureMap;
};

class GM_EXPORT GMUIConfiguration : public IDestroyObject
{
	friend class GMWidgetResourceManager;
	GM_DECLARE_PRIVATE_NGO(GMUIConfiguration)

public:
	GMUIConfiguration(const IRenderContext* context);
	GMUIConfiguration(const GMUIConfiguration&) = default;

public:
	void initResourceManager(GMWidgetResourceManager* manager);
	bool import(const GMBuffer& buffer);
	void addTexture(GMUITexture texture);
	void addArea(GMUIArea area);

private:
	void initWidget(GMWidget* widget);

public:
	inline const IRenderContext* getContext() GM_NOEXCEPT
	{
		D(d);
		return d->context;
	}
};

END_NS
#endif