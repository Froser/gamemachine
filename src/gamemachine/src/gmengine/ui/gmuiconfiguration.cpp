#include "stdafx.h"
#include "gmuiconfiguration.h"
#include "gmwidget.h"
#include "gmdata/xml/gmxml.h"
#include "foundation/utilities/tools.h"
#include "gmdata/imagereader/gmimagereader.h"
#include "foundation/gamemachine.h"

BEGIN_NS

GM_INTERFACE(IGMUIParser)
{
	virtual bool Parse(GMUIConfiguration& configuration, GMXMLElement* root) = 0;
};

template <GMint32>
class GMUIParser;

template <>
class GMUIParser<0> : public IGMUIParser
{
	virtual bool Parse(GMUIConfiguration& configuration, GMXMLElement* root) override;
};

static IGMUIParser* getParser(GMfloat version)
{
	if (version == 1.0f)
	{
		static GMUIParser<0> parser;
		return &parser;
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool GMUIParser<0>::Parse(GMUIConfiguration& configuration, GMXMLElement* root)
{
	IFactory* factory = GM.getFactory();
	GM_ASSERT(factory);

	GMXMLElement* elem = root->FirstChildElement();
	while (elem)
	{
		if (GMString::stringEquals(elem->Value(), "textures"))
		{
			// 解析textures
			GMXMLElement* textureElement = elem->FirstChildElement();
			while (textureElement)
			{
				if (!GMString::stringEquals(textureElement->Value(), "texture"))
				{
					gm_warning(gm_dbg_wrap("Unrecognized node {0} while parsing 'textures', skipped."), GMString(textureElement->Value()));
					textureElement = textureElement->NextSiblingElement(); 
					continue;
				}

				bool parsed = false;
				GMlong id = GMString::parseLong(textureElement->Attribute("id"), &parsed);
				if (!parsed)
				{
					gm_warning(gm_dbg_wrap("Unrecognized id {0}, skipped."), GMString(textureElement->Attribute("id")));
					textureElement = textureElement->NextSiblingElement(); 
					continue;
				}

				// 解析纹理
				std::string textureAssetBase64 = textureElement->GetText();
				GMBuffer textureBufferBase64 = GMBuffer::createBufferView(
					(GMbyte*)(textureAssetBase64.data()),
					textureAssetBase64.length() + 1
				);
				GMBuffer texBuffer = GMConvertion::fromBase64(textureBufferBase64);
				GMImage* image = nullptr;
				GMImageReader::load(texBuffer.getData(), texBuffer.getSize(), GMImageReader::ImageType_AUTO, &image);
				if (!image)
				{
					gm_warning(gm_dbg_wrap("Unrecognized image format, id = {0}, skipped."), GMString(id));
				}
				else
				{
					GMAsset asset;
					factory->createTexture(configuration.getContext(), image, asset);
					GM_ASSERT(!asset.isEmpty());
					if (asset.isEmpty())
					{
						gm_warning(gm_dbg_wrap("Error occurs during create texture, id = {0}, skipped."), GMString(id));
					}
					else
					{
						GMUITexture texture = { id, image->getWidth(), image->getHeight(), asset };
						configuration.addTexture(texture);
					}
				}

				GM_delete(image);
				textureElement = textureElement->NextSiblingElement();
			}
		}
		else if (GMString::stringEquals(elem->Value(), "areas"))
		{
			// 解析areas
			GMXMLElement* areaElement = elem->FirstChildElement();
			while (areaElement)
			{
				if (!GMString::stringEquals(areaElement->Value(), "area"))
				{
					gm_warning(gm_dbg_wrap("Unrecognized node {0} while parsing 'areas', skipped."), GMString(areaElement->Value()));
					areaElement = areaElement->NextSiblingElement();
					continue;
				}

				GMUIArea area;
				area.area = static_cast<GMTextureArea::Area>(GMString::parseLong(areaElement->Attribute("area")));
				area.textureId = GMString::parseLong(areaElement->Attribute("textureId"));
				area.rc.x = GMString::parseInt(areaElement->Attribute("x"));
				area.rc.y = GMString::parseInt(areaElement->Attribute("y"));
				area.rc.width = GMString::parseInt(areaElement->Attribute("width"));
				area.rc.height = GMString::parseInt(areaElement->Attribute("height"));
				area.cornerRc.x = GMString::parseInt(areaElement->Attribute("cornerX"));
				area.cornerRc.y = GMString::parseInt(areaElement->Attribute("cornerY"));
				area.cornerRc.width = GMString::parseInt(areaElement->Attribute("cornerWidth"));
				area.cornerRc.height = GMString::parseInt(areaElement->Attribute("cornerHeight"));
				configuration.addArea(area);
				areaElement = areaElement->NextSiblingElement();
			}
		}
		elem = elem->NextSiblingElement();
	}
	return true;
}


GM_PRIVATE_OBJECT_UNALIGNED(GMUIConfiguration)
{
	const IRenderContext* context;
	Vector<GMUITexture> textures;
	Vector<GMUIArea> areas;
	Map<GMlong, GMlong> textureMap;
};

GMUIConfiguration::GMUIConfiguration(const IRenderContext* context)
{
	GM_CREATE_DATA();

	D(d);
	d->context = context;
}

GMUIConfiguration::~GMUIConfiguration()
{

}

GMUIConfiguration::GMUIConfiguration(const GMUIConfiguration& rhs)
{
	*this = rhs;
}

GMUIConfiguration::GMUIConfiguration(GMUIConfiguration&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMUIConfiguration& GMUIConfiguration::operator=(const GMUIConfiguration& rhs)
{
	GM_COPY(rhs);
	return *this;
}

GMUIConfiguration& GMUIConfiguration::operator=(GMUIConfiguration&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}

void GMUIConfiguration::initResourceManager(GMWidgetResourceManager* manager)
{
	D(d);
	for (auto texture : d->textures)
	{
		d->textureMap[texture.id] = manager->addTexture(texture.asset, texture.width, texture.height);
	}
	manager->setUIConfiguration(*this);
}

void GMUIConfiguration::initWidget(GMWidget* widget)
{
	D(d);
	for (const auto& area : d->areas)
	{
		widget->addArea(area.area, d->textureMap[area.textureId], area.rc, area.cornerRc);
	}
	widget->initStyles();
}

bool GMUIConfiguration::import(const GMBuffer& buffer)
{
	GMXMLDocument xmlDocument;
	GMXMLError err = xmlDocument.Parse(reinterpret_cast<const char*>(buffer.getData()), buffer.getSize());
	if (err != GMXMLError::XML_SUCCESS)
	{
		gm_error(gm_dbg_wrap("Error in reading UI files. Reason: {0}"), xmlDocument.ErrorStr());
		return false;
	}

	GMXMLElement* root = xmlDocument.RootElement();
	if (!GMString::stringEquals(root->Value(), "gmui"))
	{
		gm_error(gm_dbg_wrap("Wrong ui format."));
		return false;
	}

	bool parseVersion;
	GMfloat version = GMString::parseFloat(root->Attribute("version"), &parseVersion);
	if (!parseVersion)
	{
		gm_error(gm_dbg_wrap("Wrong ui version."));
		return false;
	}

	IGMUIParser* parser = nullptr;
	if (!(parser = getParser(version)))
	{
		gm_error(gm_dbg_wrap("Unrecognized version {0}"), GMString(version));
		return false;
	}

	return parser->Parse(*this, root);
}

void GMUIConfiguration::addTexture(GMUITexture texture)
{
	D(d);
	d->textures.push_back(std::move(texture));
}

void GMUIConfiguration::addArea(GMUIArea area)
{
	D(d);
	d->areas.push_back(std::move(area));
}

const IRenderContext* GMUIConfiguration::getContext() GM_NOEXCEPT
{
	D(d);
	return d->context;
}

END_NS