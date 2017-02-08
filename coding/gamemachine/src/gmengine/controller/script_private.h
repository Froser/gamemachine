#ifndef __SCRIPT_PRIVATE_H__
#define __SCRIPT_PRIVATE_H__
#include "common.h"
#include <vector>
BEGIN_NS

#define STMT_RESULT GMuint

class Script;
class GameObject;
struct ScriptPrivate
{
	typedef std::vector<std::string> Expression;

	void setScript(Script* s) { script = s; }
	void parseLine(const char* line);
	void parseExpressions();

	GameObject* findObject(const char* identifier);

	STMT_RESULT stmt_first(Expression& expr);
	STMT_RESULT stmt_region(Expression& expr, Expression::iterator& iter);
	STMT_RESULT stmt_region_statements(Expression& expr, Expression::iterator& iter, GameObject* source, GameObject* dest);

	Expression expression;
	Script* script;
};

END_NS
#endif