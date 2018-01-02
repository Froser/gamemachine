#ifndef __GMSTRING_H__
#define __GMSTRING_H__
#include "../foundation/defines.h"
#include "../foundation/gmobject.h"
#include <string>
#include "assert.h"
BEGIN_NS

// 所有的字符，按照UTF-16来处理
// 如果传入的是一个窄字符，则转换为宽字符

GM_PRIVATE_OBJECT(GMString)
{
	std::wstring data;
};

static inline GMString operator +(const GMString& left, const GMString& right);

class GMString
{
	DECLARE_PRIVATE_NGO(GMString)

	friend inline GMString operator +(const GMString& left, const GMString& right);

public:
	static size_t npos;

public:
	GMString() = default;
	GMString(const GMString&);
	GMString(GMString&&) noexcept;
	GMString(const char* c);
	GMString(const GMwchar* c);
	GMString(const std::string& str);
	GMString(const std::wstring& str);
	GMString(const GMfloat f);
	GMString(const GMint i);

public:
	bool operator == (const GMString& str) const
	{
		D(d);
		return d->data == str.toStdWString();
	}

	bool operator != (const GMString& str) const
	{
		return !((*this) == str);
	}

	bool operator < (const GMString& str) const
	{
		D(d);
		return d->data < str.toStdWString();
	}

	GMString& operator = (const GMString& str)
	{
		assign(str);
		return *this;
	}

	GMString& operator = (GMString&& s) noexcept
	{
		D(d);
		using namespace std;
		swap(d->data, s.data()->data);
		return *this;
	}

	GMString& operator += (const GMString& str)
	{
		*this = *this + str;
		return *this;
	}

	GMString& operator = (const char* str)
	{
		return this->operator=(GMString(str));
	}

	GMString& operator = (const GMwchar* str)
	{
		return this->operator=(GMString(str));
	}

	char operator[](GMuint i) const;

	const GMwchar* c_str() const
	{
		D(d);
		return d->data.c_str();
	}
	
public:
	void copyString(char *s) const;
	void copyString(GMwchar *s) const;
	GMString& append(const GMwchar* c);
	GMString& append(const char* c);
	size_t findLastOf(GMwchar c) const;
	size_t findLastOf(char c) const;
	GMString substr(GMint start, GMint count) const;
	const std::wstring& toStdWString() const;
	const std::string toStdString() const;
	size_t length() const;
	GMString replace(const GMString& oldValue, const GMString& newValue);

private:
	void assign(const GMString& s);
};

inline GMString operator +(const GMString& left, const GMString& right)
{
	return left.data()->data + right.data()->data;
}

class GMStringReader
{
	class Iterator
	{
	public:
		Iterator(const Iterator& iter) : m_src(iter.m_src), m_start(iter.m_start), m_end(iter.m_end) {}

		Iterator(const GMString& string) : m_src(string)
		{
			findNextLine();
		}

	public:
		GMString operator *();

		Iterator& operator++(int)
		{
			return ++*this;
		}

		Iterator& operator++()
		{
			findNextLine();
			return *this;
		}

		bool hasNextLine()
		{
			return !m_eof;
		}

	private:
		void findNextLine();

	private:
		const GMString& m_src;
		size_t m_start = 0, m_end = 0;
		bool m_eof = false;
	};

public:
	GMStringReader(const GMString& str) : m_string(str) {}

public:
	Iterator lineBegin();

private:
	const GMString m_string;
};

END_NS
#endif
