#include "stdafx.h"
#include <stdio.h>
#include "scanner.h"

Scanner::Scanner(const char* line, CharPredicate predicate)
	: m_p(line)
	, m_predicate(predicate)
	, m_skipSame(true)
{
}

Scanner::Scanner(const char* line, bool skipSame, CharPredicate predicate)
	: m_p(line)
	, m_predicate(predicate)
	, m_skipSame(skipSame)
{

}

void Scanner::next(char* out)
{
	char* p = out;
	bool b = false;
	while (*m_p && m_predicate(*m_p))
	{
		if (b && !m_skipSame)
		{
			*p = 0;
			m_p++;
			return;
		}
		m_p++;
		b = true;
	}

	if (!*m_p)
	{
		*p = 0;
		return;
	}

	do
	{
		*p = *m_p;
		p++;
		m_p++;
	} while (*m_p && !m_predicate(*m_p));

	*p = 0;
}

void Scanner::nextToTheEnd(char* out)
{
	char* p = out;
	while (*m_p)
	{
		m_p++;
		*p = *m_p;
		p++;
	}
}

bool Scanner::nextFloat(Ffloat* out)
{
	char command[LINE_MAX];
	next(command);
	if (!strlen(command))
		return false;
	sscanf_s(command, "%f", out);
	return true;
}

bool Scanner::nextInt(Fint* out)
{
	char command[LINE_MAX];
	next(command);
	if (!strlen(command))
		return false;
	sscanf_s(command, "%i", out);
	return true;
}
