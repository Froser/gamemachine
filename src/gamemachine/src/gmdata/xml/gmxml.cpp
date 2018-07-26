#include "stdafx.h"
#include "gmxml.h"

namespace
{
	void parseMember(const GMXMLElement* element, GMObject& object)
	{
		if (!element)
			return;

		const GMMeta* meta = object.meta();
		for (auto member : *meta)
		{
			// 获取所有成员，进行深度遍历
			const GMXMLElement* elementMember = element->NextSiblingElement(member.first.toStdString().c_str());
			GMString value = elementMember->Value();
			switch (member.second.type)
			{
			case GMMetaMemberType::Boolean:
			{
				*static_cast<bool*>(member.second.ptr) = value != "false";
				break;
			}
			case GMMetaMemberType::String:
			{
				*static_cast<GMString*>(member.second.ptr) = value;
				break;
			}
			case GMMetaMemberType::Float:
			{
				*static_cast<GMfloat*>(member.second.ptr) = GMString::parseFloat(value);
				break;
			}
			case GMMetaMemberType::Int:
			{
				*static_cast<GMint*>(member.second.ptr) = GMString::parseInt(value);
				break;
			}
			case GMMetaMemberType::Object:
			{
				parseMember(elementMember->FirstChildElement(), *static_cast<GMObject*>(member.second.ptr));
				break;
			}
			}
		}
	}

	void assignMember(const GMMeta* meta, const GMString& name, const GMString& plistType, const GMString& plistValue)
	{
		for (auto member : *meta)
		{
			if (member.first == name)
			{
				if (plistType == "real")
				{
					if (member.second.type == GMMetaMemberType::Float)
						*static_cast<GMfloat*>(member.second.ptr) = GMString::parseFloat(plistValue);
					else if (member.second.type == GMMetaMemberType::Int)
						*static_cast<GMint*>(member.second.ptr) = static_cast<GMint>(GMString::parseFloat(plistValue));
				}
				else if (plistType == "integer")
				{
					if (member.second.type == GMMetaMemberType::Float)
						*static_cast<GMfloat*>(member.second.ptr) = static_cast<GMfloat>(GMString::parseInt(plistValue));
					else if (member.second.type == GMMetaMemberType::Int)
						*static_cast<GMint*>(member.second.ptr) = GMString::parseInt(plistValue);
				}
				else if (plistType == "string")
				{
					*static_cast<GMString*>(member.second.ptr) = plistValue;
				}
				else
				{
					gm_warning(gm_dbg_wrap("plist type {0} not support."), plistType);
				}
			}
		}
	}
}

bool GMXML::parseXML(const GMString& content, GMObject& object)
{
	using namespace tinyxml2;

	GMXMLDocument doc;
	auto result = doc.Parse(content.toStdString().c_str());
	if (result != XML_SUCCESS)
	{
		gm_error(gm_dbg_wrap("{0}"), doc.ErrorStr());
		return false;
	}

	GMXMLElement* root = doc.RootElement();
	parseMember(root->FirstChildElement(), object);
	return true;
}

bool GMXML::parsePlist(const GMString& content, GMObject& object)
{
	using namespace tinyxml2;

	GMXMLDocument doc;
	auto result = doc.Parse(content.toStdString().c_str());
	if (result != XML_SUCCESS)
	{
		gm_error(gm_dbg_wrap("{0}"), doc.ErrorStr());
		return false;
	}

	GMXMLElement* plist = doc.RootElement();
	GMXMLElement* dict = plist->FirstChildElement("dict");
	if (dict)
	{
		GMXMLElement* pointer = dict->FirstChildElement();
		while (pointer)
		{
			GMString key;
			while (pointer && GMString::stringEquals(pointer->Value(), "key"))
			{
				key = pointer->GetText();
				pointer = pointer->NextSiblingElement();
			}

			if (!pointer)
			{
				gm_error(gm_dbg_wrap("Invalid plist file."));
				return false;
			}

			GMString type = pointer->Name();
			GMString value = pointer->GetText();
			assignMember(object.meta(), key, type, value);
			pointer = pointer->NextSiblingElement();
		}
	}
	return true;
}