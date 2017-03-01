#include "stdafx.h"
#include <stdio.h>
#include "scanner.h"

static bool isWhiteSpace(char c)
{
	return !!isspace(c);
}

Scanner::Scanner(const char* line)
	: m_p(line)
	, m_predicate(isWhiteSpace)
	, m_skipSame(true)
{
	m_valid = !!m_p;
}

Scanner::Scanner(const char* line, CharPredicate predicate)
	: m_p(line)
	, m_predicate(predicate)
	, m_skipSame(true)
{
	m_valid = !!m_p;
}

Scanner::Scanner(const char* line, bool skipSame, CharPredicate predicate)
	: m_p(line)
	, m_predicate(predicate)
	, m_skipSame(skipSame)
{
	m_valid = !!m_p;
}

void Scanner::next(char* out)
{
	if (!m_valid)
	{
		strcpy(out, "");
		return;
	}

	char* p = out;
	bool b = false;
	if (!m_p)
	{
		strcpy(out, "");
		return;
	}

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
	if (!m_valid)
		return;

	char* p = out;
	while (*m_p)
	{
		m_p++;
		*p = *m_p;
		p++;
	}
}

bool Scanner::nextFloat(GMfloat* out)
{
	if (!m_valid)
		return false;

	char command[LINE_MAX];
	next(command);
	if (!strlen(command))
		return false;
	sscanf_s(command, "%f", out);
	return true;
}

bool Scanner::nextInt(GMint* out)
{
	if (!m_valid)
		return false;

	char command[LINE_MAX];
	next(command);
	if (!strlen(command))
		return false;
	sscanf_s(command, "%i", out);
	return true;
}
