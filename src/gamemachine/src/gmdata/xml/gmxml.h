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

END_NS
#endif