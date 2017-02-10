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

	STMT_RESULT stmt_first(Expression& expr, Expression::iterator& iter);
	STMT_RESULT stmt_reach(Expression& expr, Expression::iterator& iter, bool reached);
	STMT_RESULT stmt_reach_statements(Expression& expr, Expression::iterator& iter, GameObject* source, GameObject* dest, bool reached);

	Expression expression;
	Script* script;
};

END_NS
#endif