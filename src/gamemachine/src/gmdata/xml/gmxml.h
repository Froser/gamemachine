#ifndef __GMXML_H__
#define __GMXML_H__
#include <gmcommon.h>

#include "tinyxml2/tinyxml2.h"

BEGIN_NS

typedef tinyxml2::XMLDocument GMXMLDocument;
typedef tinyxml2::XMLElement GMXMLElement;
typedef tinyxml2::XMLAttribute GMXMLAttribute;
typedef tinyxml2::XMLComment GMXMLComment;
typedef tinyxml2::XMLText GMXMLText;
typedef tinyxml2::XMLDeclaration GMXMLDeclaration;
typedef tinyxml2::XMLError GMXMLError;

class GMXML
{
public:
	static bool parseXML(const GMString& content, GMObject& object);
	static bool parsePlist(const GMString& content, GMObject& object);
};

END_NS
#endif