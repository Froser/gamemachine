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

GM_PRIVATE_CLASS(GMUIConfiguration);
class GM_EXPORT GMUIConfiguration : public IDestroyObject
{
	GM_FRIEND_CLASS(GMWidgetResourceManager);
	GM_DECLARE_PRIVATE(GMUIConfiguration)

public:
	GMUIConfiguration(const IRenderContext* context);
	~GMUIConfiguration();
	GMUIConfiguration(const GMUIConfiguration&);
	GMUIConfiguration(GMUIConfiguration&&) GM_NOEXCEPT;
	GMUIConfiguration& operator=(const GMUIConfiguration& rhs);
	GMUIConfiguration& operator=(GMUIConfiguration&& rhs) GM_NOEXCEPT;

public:
	void initResourceManager(GMWidgetResourceManager* manager);
	bool import(const GMBuffer& buffer);
	void addTexture(GMUITexture texture);
	void addArea(GMUIArea area);
	const IRenderContext* getContext() GM_NOEXCEPT;

private:
	void initWidget(GMWidget* widget);
};

END_NS
#endif