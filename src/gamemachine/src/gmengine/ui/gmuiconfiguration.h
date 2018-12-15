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

GM_PRIVATE_OBJECT(GMUIConfiguration)
{
	const IRenderContext* context;
	Vector<GMUITexture> textures;
	Vector<GMUIArea> areas;
	Map<GMlong, GMlong> textureMap;
};

class GM_EXPORT GMUIConfiguration : public GMObject
{
	GM_DECLARE_PRIVATE(GMUIConfiguration)

public:
	GMUIConfiguration(const IRenderContext* context);

public:
	void initResourceManager(GMWidgetResourceManager* manager);
	void initWidget(GMWidget* widget);
	bool import(const GMBuffer& buffer);
	void addTexture(GMUITexture texture);
	void addArea(GMUIArea area);

public:
	inline const IRenderContext* getContext() GM_NOEXCEPT
	{
		D(d);
		return d->context;
	}
};

END_NS
#endif