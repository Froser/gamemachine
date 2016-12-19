#ifndef __AUTOPTR_H__
#define __AUTOPTR_H__
#include "common.h"

BEGIN_NS
template<class Type>
struct AutoPtrRef
{
	explicit AutoPtrRef(Type *right)
		: m_ref(right)
	{
	}

	Type *m_ref;
};

template<class Type>
class AutoPtr
{
public:
	typedef AutoPtr<Type> MyType;
	typedef Type ElementType;

	explicit AutoPtr(Type *ptr = nullptr)
		: m_ptr(ptr)
	{
	}

	AutoPtr(MyType& right)
		: m_ptr(right.release())
	{
	}

	AutoPtr(AutoPtrRef<Type> right)
	{
		Type *ptr = right.m_ref;
		right.m_ref = 0;
		m_ptr = ptr;
	}

	template<class OtherType>
	operator AutoPtr<OtherType>()
	{
		return (AutoPtr<OtherType>(*this));
	}

	template<class OtherType>
	operator AutoPtrRef<OtherType>()
	{
		OtherType *Cvtptr = m_ptr;
		AutoPtrRef<OtherType> _Ans(Cvtptr);
		m_ptr = 0;
		return (_Ans);
	}

	template<class OtherType>
	MyType& operator=(AutoPtr<OtherType>& right)
	{
		reset(right.release());
		return (*this);
	}

	template<class OtherType>
	AutoPtr(AutoPtr<OtherType>& right)
		: m_ptr(right.release())
	{
	}

	MyType& operator=(MyType& right)
	{
		reset(right.release());
		return (*this);
	}

	MyType& operator=(AutoPtrRef<Type> right)
	{
		Type *ptr = right.m_ref;
		right.m_ref = 0;
		reset(ptr);
		return (*this);
	}

	~AutoPtr()
	{
		delete m_ptr;
	}

	Type& operator*() const
	{
		return (*get());
	}

	Type *operator->() const
	{
		return (get());
	}

	Type *get() const
	{
		return (m_ptr);
	}

	Type *release()
	{
		Type *_Tmp = m_ptr;
		m_ptr = 0;
		return (_Tmp);
	}

	operator Type *()
	{
		return (m_ptr);
	}

	operator bool()
	{
		return !!m_ptr;
	}

	void reset(Type *ptr = nullptr)
	{
		if (ptr != m_ptr)
			delete m_ptr;
		m_ptr = ptr;
	}

private:
	Type *m_ptr;
};
END_NS

#endif