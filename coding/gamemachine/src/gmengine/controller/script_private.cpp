#include "stdafx.h"
#include "script_private.h"
#include "utilities/scanner.h"
#include "gmengine/elements/gameobject.h"
#include "gmengine/elements/character.h"
#include "script.h"

#define SCRIPT_OK 0
#define SCRIPT_FAILED 1
#define TOKEN_EQUAL(iter, cstr) strEqual((*iter).c_str(), cstr)
#define SCRIPT_IF(expr) if (!(expr)) return SCRIPT_FAILED; else
#define CHECK_TOKEN_AND_MOVE_FORWARD(iter, token) SCRIPT_IF(TOKEN_EQUAL(iter, token)) iter++;

// Keywords:
#define BEGIN "begin"
#define REACHED "reached"
#define UNREACHED "unreached"
#define CHARACTER "character"
#define END "end"

static bool isSeparator(char c)
{
	return !!isspace(c)
		|| c == '('
		|| c == ','
		|| c == ')'
		;
}

void ScriptPrivate::parseLine(const char* line)
{
	Scanner scanner(line, isSeparator);

	char token[LINE_MAX] = { 0 };
	scanner.next(token);
	while (!strEqual(token, ""))
	{
		expression.push_back(token);
		scanner.next(token);
	}
}

void ScriptPrivate::parseExpressions()
{
	auto begin = expression.begin();
	while (stmt_first(expression, begin) == SCRIPT_OK)
	{
	};
}

GameObject* ScriptPrivate::findObject(const char* identifier)
{
	if (strEqual(identifier, CHARACTER))
		return script->getWorld()->getMajorCharacter();

	GMuint id;
	SAFE_SSCANF(identifier, "%i", &id);
	return script->getWorld()->findGameObjectById(id);
}

STMT_RESULT ScriptPrivate::stmt_first(Expression& expr, Expression::iterator& iter)
{
	if (iter == expr.end())
		return SCRIPT_FAILED;

	if (TOKEN_EQUAL(iter, REACHED))
		return stmt_reach(expr, ++iter, true);

	if (TOKEN_EQUAL(iter, UNREACHED))
		return stmt_reach(expr, ++iter, false);

	return SCRIPT_FAILED;
}

STMT_RESULT ScriptPrivate::stmt_reach(Expression& expr, Expression::iterator& iter, bool reached)
{
	std::string source, target;
	SCRIPT_IF(iter != expr.end())
	{
		source = *(iter++);
	}

	SCRIPT_IF(iter != expr.end())
	{
		target = *(iter++);
	}

	CHECK_TOKEN_AND_MOVE_FORWARD(iter, BEGIN);
	GameObject* sourceObj = findObject(source.c_str()), *targetObj = findObject(target.c_str());
	stmt_reach_statements(expr, iter, sourceObj, targetObj, reached);
	CHECK_TOKEN_AND_MOVE_FORWARD(iter, END);
	return SCRIPT_OK;
}

STMT_RESULT ScriptPrivate::stmt_reach_statements(Expression& expr, Expression::iterator& iter, GameObject* source, GameObject* dest, bool reached)
{
	std::vector<EventItem::Action> actions;
	while (!TOKEN_EQUAL(iter, END) )
	{
		EventItem::Action action;

		bool first = true;
		while (!TOKEN_EQUAL(iter, ";"))
		{
			if (first)
				action.name = *iter;
			else
				action.args.push_back(*iter);
			first = false;
			iter++;
		}
		actions.push_back(action);
		iter++;
	}

	EventItem evt = {
		reached ? EventItem::Reached : EventItem::Unreached,
		dest,
		actions
	};

	source->addEvent(evt);
	return SCRIPT_OK;
}