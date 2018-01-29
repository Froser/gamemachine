#ifndef __GMCOM_H__
#define __GMCOM_H__
#include <gamemachine.h>

#if GM_WINDOWS
#include <Unknwnbase.h>

#define GM_COM_CHECK(hr) if (FAILED(hr)) { GM_ASSERT(false); return; }

BEGIN_NS
template <class E>
class GMComPtr
{
public:
	GMComPtr() { m_ptr = NULL; }
	GMComPtr(E* p)
	{
		m_ptr = p;
		if (m_ptr)
			m_ptr->AddRef();
	}
	GMComPtr(const GMComPtr<E>& p)
	{
		m_ptr = p.m_ptr;
		if (m_ptr)
			m_ptr->AddRef();
	}
	GMComPtr(IUnknown* pUnk) : m_ptr(NULL)
	{
		if (pUnk)
			pUnk->QueryInterface(__uuid(E), (void**)&m_ptr);
	}

	GMComPtr(IUnknown* pUnk, REFIID iid) : m_ptr(NULL)
	{
		if (pUnk)
			pUnk->QueryInterface(iid, (void**)&m_ptr);
	}

	template <class Type>
	GMComPtr(const GMComPtr<Type>& p) : m_ptr(NULL)
	{
		if (p)
			p->QueryInterface(__uuid(E), (void**)&m_ptr);
	}

	~GMComPtr()
	{
		if (m_ptr)
			m_ptr->Release();
	}

	void attach(E* p)
	{
		if (m_ptr)
			m_ptr->Release();
		m_ptr = p;
	}

	void detach(E** ppv)
	{
		GM_ASSERT(0); // 过时的用法!
		GM_ASSERT(ppv != &m_ptr);
		*ppv = m_ptr;
		m_ptr = NULL;
	}

	E* detach()
	{
		E* tmp = m_ptr;
		m_ptr = NULL;
		return tmp;
	}

	void clear()
	{
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = NULL;
		}
	}

	E* operator=(E* p)
	{
		if (p)
			p->AddRef();
		if (m_ptr)
			m_ptr->Release();
		return m_ptr = p;
	}
	E* operator=(const GMComPtr<E>& p)
	{
		if (p.m_ptr)
			p.m_ptr->AddRef();
		if (m_ptr)
			m_ptr->Release();
		return m_ptr = p.m_ptr;
	}

	E* operator=(int __nil)
	{
		GM_ASSERT(__nil == 0);
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = NULL;
		}
		return NULL;
	}

	E* operator=(long __nil)
	{
		GM_ASSERT(__nil == 0);
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = NULL;
		}
		return NULL;
	}
	E* operator=(IUnknown* pUnk)
	{
		E* pTemp = NULL;
		if (pUnk)
			pUnk->QueryInterface(__uuid(E), (void**)&pTemp);
		if (m_ptr)
			m_ptr->Release();
		return m_ptr = pTemp;
	}

	template <class Type>
	E* operator=(const GMComPtr<Type>& p)
	{
		E* pTemp = NULL;
		if (p)
			p->QueryInterface(__uuid(E), (void**)&pTemp);
		if (m_ptr)
			m_ptr->Release();
		return m_ptr = pTemp;
	}

	BOOL IsEqualObject(IUnknown* pOther) const
	{
		if (pOther)
		{
			if (m_ptr == NULL)
				return FALSE;
			IUnknown* pThis;
			m_ptr->QueryInterface(IID_IUnknown, (void**)&pThis);
			pOther->QueryInterface(IID_IUnknown, (void**)&pOther);
			pThis->Release();
			pOther->Release();
			return (pThis == pOther);
		}
		return (m_ptr == NULL);
	}

	E& operator*() const
	{
		return *m_ptr;
	}

	operator E*() const
	{
		return m_ptr;
	}

	E* get() const
	{
		return m_ptr;
	}

	E** operator&()
	{
		return &m_ptr;
	}

	E* operator->() const
	{
		return m_ptr;
	}

	BOOL operator!() const
	{
		return m_ptr == nullptr;
	}

	BOOL operator==(E* p) const
	{
		return m_ptr == p;
	}

	BOOL operator!=(E* p) const
	{
		return m_ptr != p;
	}

protected:
	E* m_ptr;
};

#endif
END_NS

#endif
