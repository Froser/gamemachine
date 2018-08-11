#ifndef __VECTOR_H__
#define __VECTOR_H__
#include <gmcommon.h>
#include "assert.h"
#include "memory.h"
#include "debug.h"

BEGIN_NS

#define GM_USE_MEMCPY 0

#include <new>
#include <xutility>

template <typename T>
class AlignedVectorIteratorBase
{
public:
	AlignedVectorIteratorBase()
		: m_ptr(nullptr) //invalid pointer
	{
	}

	AlignedVectorIteratorBase(typename T::value_type* ptr)
		: m_ptr(ptr)
	{
	}

	AlignedVectorIteratorBase(const AlignedVectorIteratorBase& other)
	{
		*this = other;
	}

	typename T::value_type& operator ++()
	{
		m_ptr++;
		return *m_ptr;
	}

	typename T::value_type& operator ++(int)
	{
		T::value_type& elem = *m_ptr;
		m_ptr++;
		return elem;
	}

	typename T::value_type& operator +=(GMptrdiff step)
	{
		m_ptr += step;
		T::value_type& elem = *m_ptr;
		return elem;
	}

	typename T::value_type& operator +(GMptrdiff step)
	{
		return *(m_ptr + step);
	}

	GMptrdiff operator-(AlignedVectorIteratorBase& other)
	{
		return other.m_ptr - m_ptr;
	}

	bool operator ==(const AlignedVectorIteratorBase& other)
	{
		return m_ptr == other.m_ptr;
	}

	bool operator !=(const AlignedVectorIteratorBase& other)
	{
		return m_ptr != other.m_ptr;
	}

	T::value_type& operator->()
	{
		return *m_ptr;
	}

	AlignedVectorIteratorBase& operator=(const AlignedVectorIteratorBase& other)
	{
		m_ptr = other.m_ptr;
		return *this;
	}

protected:
	typename T::value_type* m_ptr;
};

template <typename T>
class AlignVectorIterator : public AlignedVectorIteratorBase<T>
{
	typedef AlignedVectorIteratorBase<T> Base;

public:
	using Base::Base;

	typename T::value_type& operator *()
	{
		return *m_ptr;
	}
};

template <typename T>
class AlignVectorConstIterator : public AlignedVectorIteratorBase<T>
{
	typedef AlignedVectorIteratorBase<T> Base;

public:
	using Base::Base;

	const typename T::value_type& operator *()
	{
		return *m_ptr;
	}
};

template <typename T>
class AlignedVector
{
public:
	typedef AlignVectorIterator<AlignedVector> iterator;
	typedef AlignVectorConstIterator<AlignedVector> const_iterator;
	typedef T value_type;

public:
	AlignedVector<value_type>& operator=(const AlignedVector<value_type> &other)
	{
		copyFromArray(other);
		return *this;
	}

protected:
	GMint allocSize(GMint size)
	{
		return (size ? size * 2 : 1);
	}

	void copy(GMint start, GMint end, value_type* dest) const
	{
		GMint i;
		for (i = start; i < end; ++i)
		{
			new (&dest[i]) value_type(m_data[i]);
		}
	}

	void init()
	{
		m_ownsMemory = true;
		m_data = 0;
		m_size = 0;
		m_capacity = 0;
	}
	void destroy(GMint first, GMint last)
	{
		GMint i;
		for (i = first; i < last; i++)
		{
			m_data[i].~value_type();
		}
	}

	void* allocate(GMint size)
	{
		if (size)
			return m_allocator.allocate(size);
		return 0;
	}

	void deallocate()
	{
		if (m_data) {
			if (m_ownsMemory)
			{
				m_allocator.deallocate(m_data);
			}
			m_data = 0;
		}
	}

public:
	AlignedVector()
	{
		init();
	}

	~AlignedVector()
	{
		clear();
	}

	AlignedVector(const AlignedVector& otherArray)
	{
		init();

		GMint otherSize = otherArray.size();
		resize(otherSize);
		otherArray.copy(0, otherSize, m_data);
	}

	/// return the number of elements in the array
	GMuint size() const
	{
		return m_size;
	}

	bool empty()
	{
		return m_size == 0;
	}

	const value_type& at(GMuint n) const
	{
		GM_ASSERT(n >= 0);
		GM_ASSERT(n < size());
		return m_data[n];
	}

	value_type& at(GMuint n)
	{
		GM_ASSERT(n >= 0);
		GM_ASSERT(n < size());
		return m_data[n];
	}

	value_type* data()
	{
		return m_data;
	}

	const value_type& operator[](GMuint n) const
	{
		GM_ASSERT(n >= 0);
		GM_ASSERT(n < size());
		return m_data[n];
	}

	value_type& operator[](GMuint n)
	{
		GM_ASSERT(n >= 0);
		GM_ASSERT(n < size());
		return m_data[n];
	}

	void clear()
	{
		destroy(0, size());
		deallocate();
		init();
	}

	void pop_back()
	{
		GM_ASSERT(m_size > 0);
		m_size--;
		m_data[m_size].~value_type();
	}

	void assign(GMsize_t count, const value_type& val)
	{
		clear();
		for (GMsize_t i = 0; i < count; ++i)
		{
			push_back(val);
		}
	}

	value_type& back()
	{
		GM_ASSERT(m_size > 0);
		return m_data[m_size - 1];
	}

	value_type& front()
	{
		GM_ASSERT(m_size > 0);
		return *m_data;
	}

	iterator find(const value_type& target)
	{
		iterator iter = begin(), e = end();
		while (iter != e)
		{
			if (target == *iter)
				return iter;
			iter++;
		}
		return iter;
	}

	void resizeNoInitialize(GMint newsize)
	{
		GMint curSize = size();

		if (newsize < curSize)
		{
		}
		else
		{
			if (newsize > size())
			{
				reserve(newsize);
			}
		}
		m_size = newsize;
	}

	void resize(GMuint newsize, const value_type& fillData = value_type())
	{
		GMuint curSize = size();
		if (newsize < curSize)
		{
			for (GMuint i = newsize; i < curSize; i++)
			{
				m_data[i].~value_type();
			}
		}
		else
		{
			if (newsize > size())
			{
				reserve(newsize);
			}
			for (GMuint i = curSize; i < newsize; i++)
			{
				new (&m_data[i]) value_type(fillData);
			}
		}

		m_size = newsize;
	}

	value_type& expandNonInitializing()
	{
		GMuint sz = size();
		if (sz == capacity())
		{
			reserve(allocSize(size()));
		}
		m_size++;

		return m_data[sz];
	}

	value_type& expand(const value_type& fillValue = value_type())
	{
		GMuint sz = size();
		if (sz == capacity())
		{
			reserve(allocSize(size()));
		}
		m_size++;
		new (&m_data[sz]) value_type(fillValue); //use the in-place new (not really allocating heap memory)
		return m_data[sz];
	}

	void push_back(const value_type& _Val)
	{
		GMuint sz = size();
		value_type __Val; // 重新分配空间的时候，如果_Val来自于自身，有可能会被销毁，这个时候应该拷贝一次
		if (sz == capacity())
		{
			__Val = _Val;
			reserve(allocSize(size()));
			new (&m_data[m_size]) value_type(std::move(__Val));
		}
		else
		{
			new (&m_data[m_size]) value_type(_Val);
		}

		m_size++;
	}

	void push_back(value_type&& _Val)
	{
		GMuint sz = size();
		value_type __Val; // 重新分配空间的时候，如果_Val来自于自身，有可能会被销毁，这个时候应该拷贝一次
		if (sz == capacity())
		{
			__Val = std::move(_Val);
			reserve(allocSize(size()));
			new (&m_data[m_size]) value_type(std::move(__Val));
		}
		else
		{
			new (&m_data[m_size]) value_type(std::move(_Val));
		}

		m_size++;
	}

	iterator begin()
	{
		iterator iter(m_data) ;
		return iter;
	}

	iterator end()
	{
		iterator iter(m_data + m_size);
		return iter;
	}

	const_iterator begin() const
	{
		const_iterator iter(m_data);
		return iter;
	}

	const_iterator end() const
	{
		const_iterator iter(m_data + m_size);
		return iter;
	}

	/// return the pre-allocated (reserved) elements, this is at least as large as the total number of elements,see size() and reserve()
	GMuint capacity() const
	{
		return m_capacity;
	}

	void reserve(GMuint _Count)
	{	// determine new minimum length of allocated storage
		if (capacity() < _Count)
		{	// not enough room, reallocate
			value_type* s = (value_type*)allocate(_Count);
			GM_ASSERT(s);
			if (s == 0)
			{
				_Count = 0;
				m_size = 0;
			}
			copy(0, size(), s);

			destroy(0, size());

			deallocate();

			m_ownsMemory = true;
			m_data = s;
			m_capacity = _Count;
		}
	}

	template <typename L>
	void quickSortInternal(const L& CompareFunc, GMint lo, GMint hi)
	{
		//  lo is the lower index, hi is the upper index
		//  of the region of array a that is to be sorted
		GMint i = lo, j = hi;
		value_type x = m_data[(lo + hi) / 2];

		//  partition
		do
		{
			while (CompareFunc(m_data[i], x))
				i++;
			while (CompareFunc(x, m_data[j]))
				j--;
			if (i <= j)
			{
				swap(i, j);
				i++; j--;
			}
		} while (i <= j);

		//  recursion
		if (lo < j)
			quickSortInternal(CompareFunc, lo, j);
		if (i < hi)
			quickSortInternal(CompareFunc, i, hi);
	}


	template <typename L>
	void quickSort(const L& CompareFunc)
	{
		//don't sort 0 or 1 elements
		if (size() > 1)
		{
			quickSortInternal(CompareFunc, 0, size() - 1);
		}
	}


	///heap sort from http://www.csse.monash.edu.au/~lloyd/tildeAlgDS/Sort/Heap/
	template <typename L>
	void downHeap(T *pArr, GMint k, GMint n, const L& CompareFunc)
	{
		/*  PRE: a[k+1..N] is a heap */
		/* POST:  a[k..N]  is a heap */

		T temp = pArr[k - 1];
		/* k has child(s) */
		while (k <= n / 2)
		{
			GMint child = 2 * k;

			if ((child < n) && CompareFunc(pArr[child - 1], pArr[child]))
			{
				child++;
			}
			/* pick larger child */
			if (CompareFunc(temp, pArr[child - 1]))
			{
				/* move child up */
				pArr[k - 1] = pArr[child - 1];
				k = child;
			}
			else
			{
				break;
			}
		}
		pArr[k - 1] = temp;
	} /*downHeap*/

	void swap(GMint index0, GMint index1)
	{
#ifdef GM_USE_MEMCPY
		char	temp[sizeof(value_type)];
		memcpy(temp, &m_data[index0], sizeof(value_type));
		memcpy(&m_data[index0], &m_data[index1], sizeof(value_type));
		memcpy(&m_data[index1], temp, sizeof(value_type));
#else
		value_type temp = m_data[index0];
		m_data[index0] = m_data[index1];
		m_data[index1] = temp;
#endif
	}

	void swap(AlignedVector<value_type>& rhs)
	{
		GM_SWAP(m_data, rhs.m_data);
		GM_SWAP(m_ownsMemory, rhs.m_ownsMemory);
		GM_SWAP(m_capacity, rhs.m_capacity);
		GM_SWAP(m_size, rhs.m_size);
	}

	template <typename L>
	void heapSort(const L& CompareFunc)
	{
		/* sort a[0..N-1],  N.B. 0 to N-1 */
		GMint k;
		GMint n = m_size;
		for (k = n / 2; k > 0; k--)
		{
			downHeap(m_data, k, n, CompareFunc);
		}

		/* a[1..N] is now a heap */
		while (n >= 1)
		{
			swap(0, n - 1); /* largest of a[0..n-1] */


			n = n - 1;
			/* restore a[1..i-1] heap */
			downHeap(m_data, 1, n, CompareFunc);
		}
	}

	GMint findBinarySearch(const value_type& key) const
	{
		GMint first = 0;
		GMint last = size() - 1;

		//assume sorted array
		while (first <= last) {
			GMint mid = (first + last) / 2;  // compute mid point.
			if (key > m_data[mid])
				first = mid + 1;  // repeat search in top half.
			else if (key < m_data[mid])
				last = mid - 1; // repeat search in bottom half.
			else
				return mid;     // found it. return position /////
		}
		return size();    // failed to find key
	}


	GMint findLinearSearch(const value_type& key) const
	{
		GMint index = size();
		GMint i;

		for (i = 0; i < size(); i++)
		{
			if (m_data[i] == key)
			{
				index = i;
				break;
			}
		}
		return index;
	}

	GMint findLinearSearch2(const value_type& key) const
	{
		GMint index = -1;
		GMint i;

		for (i = 0; i < size(); i++)
		{
			if (m_data[i] == key)
			{
				index = i;
				break;
			}
		}
		return index;
	}

	void remove(const value_type& key)
	{

		GMint findIndex = findLinearSearch(key);
		if (findIndex < size())
		{
			swap(findIndex, size() - 1);
			pop_back();
		}
	}

	void initializeFromBuffer(void *buffer, GMint size, GMint capacity)
	{
		clear();
		m_ownsMemory = false;
		m_data = (T*)buffer;
		m_size = size;
		m_capacity = capacity;
	}

	void copyFromArray(const AlignedVector& otherArray)
	{
		GMint otherSize = otherArray.size();
		resize(otherSize);
		otherArray.copy(0, otherSize, m_data);
	}

private:
	AlignedAllocator<value_type, 16>	m_allocator;
	GMint m_size;
	GMint m_capacity;
	value_type* m_data;
	bool m_ownsMemory;
};

END_NS

namespace std
{
	template <typename T>
	struct iterator_traits<gm::AlignVectorIterator<T>>
	{
		typedef random_access_iterator_tag iterator_category;
	};
}

#endif
