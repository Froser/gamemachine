#ifndef __SCANNER_H__
#define __SCANNER_H__
#include "common.h"
BEGIN_NS

typedef bool(*CharPredicate)(char in);

class Scanner
{
public:
	explicit Scanner(const char* line);
	explicit Scanner(const char* line, CharPredicate predicate);
	explicit Scanner(const char* line, bool skipSame, CharPredicate predicate);

public:
	void next(char* out);
	void nextToTheEnd(char* out);
	bool nextFloat(GMfloat* out);
	bool nextInt(GMint* out);

private:
	const char* m_p;
	bool m_skipSame;
	CharPredicate m_predicate;
	bool m_valid;
};

END_NS
#endif