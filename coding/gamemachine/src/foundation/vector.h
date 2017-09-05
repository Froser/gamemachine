#ifndef __VECTOR_H__
#define __VECTOR_H__
#include <gmcommon.h>
#include "assert.h"
#include "memory.h"
#include "debug.h"

#if !USE_SIMD
#	include <algorithm>
#endif

BEGIN_NS

#if !USE_SIMD
template <typename T>
class AlignedVector : public Vector<T>
{
	typedef std::vector<T> Base;
	
public:
	typename Base::iterator find(const T& target)
	{
		return std::find(Base::begin(), Base::end(), target);
	}
};

#else
#	define GM_USE_PLACEMENT_NEW 1
#	define GM_ALLOW_ARRAY_COPY_OP
#	define GM_USE_MEMCPY 0

#	ifdef GM_USE_PLACEMENT_NEW
#		include <new>
#	endif

template <typename T>
class AlignedVectorIteratorBase
{
public:
	AlignedVectorIteratorBase(typename T::value_type* ptr)
		: m_ptr(ptr)
	{
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

	bool operator ==(const AlignedVectorIteratorBase& other)
	{
		return m_ptr == other.m_ptr;
	}

	bool operator !=(const AlignedVectorIteratorBase& other)
	{
		return m_ptr != other.m_ptr;
	}

protected:
	typename T::value_type* m_ptr;
};

template <typename T>
class AlignVectorIterator : public AlignedVectorIteratorBase<T>
{
	typedef AlignedVectorIteratorBase<T> Base;

public:
	AlignVectorIterator(typename T::value_type* ptr) : Base(ptr) {}

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
	AlignVectorConstIterator(typename T::value_type* ptr) : Base(ptr) {}

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

#ifdef GM_ALLOW_ARRAY_COPY_OP
public:
	AlignedVector<T>& operator=(const AlignedVector<T> &other)
	{
		copyFromArray(other);
		return *this;
	}
#else//GM_ALLOW_ARRAY_COPY_OP
private:
	AlignedVector<T>& operator=(const AlignedVector<T> &other);
#endif//GM_ALLOW_ARRAY_COPY_OP

protected:
	GMint allocSize(GMint size)
	{
		return (size ? size * 2 : 1);
	}

	void copy(GMint start, GMint end, T* dest) const
	{
		GMint i;
		for (i = start; i < end; ++i)
#ifdef GM_USE_PLACEMENT_NEW
			new (&dest[i]) T(m_data[i]);
#else
			dest[i] = m_data[i];
#endif //GM_USE_PLACEMENT_NEW
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
			m_data[i].~T();
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

	const T& at(GMuint n) const
	{
		GM_ASSERT(n >= 0);
		GM_ASSERT(n < size());
		return m_data[n];
	}

	T& at(GMuint n)
	{
		GM_ASSERT(n >= 0);
		GM_ASSERT(n < size());
		return m_data[n];
	}

	T* data()
	{
		return m_data;
	}

	const T& operator[](GMuint n) const
	{
		GM_ASSERT(n >= 0);
		GM_ASSERT(n < size());
		return m_data[n];
	}

	T& operator[](GMuint n)
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
		m_data[m_size].~T();
	}

	T& back()
	{
		GM_ASSERT(m_size > 0);
		return m_data[m_size - 1];
	}

	T& front()
	{
		GM_ASSERT(m_size > 0);
		return *m_data;
	}

	iterator find(const T& target)
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

	void resize(GMuint newsize, const T& fillData = T())
	{
		GMuint curSize = size();
		if (newsize < curSize)
		{
			for (GMuint i = newsize; i < curSize; i++)
			{
				m_data[i].~T();
			}
		}
		else
		{
			if (newsize > size())
			{
				reserve(newsize);
			}
#ifdef GM_USE_PLACEMENT_NEW
			for (GMuint i = curSize; i < newsize; i++)
			{
				new (&m_data[i]) T(fillData);
			}
#endif //GM_USE_PLACEMENT_NEW
		}

		m_size = newsize;
	}

	T& expandNonInitializing()
	{
		GMuint sz = size();
		if (sz == capacity())
		{
			reserve(allocSize(size()));
		}
		m_size++;

		return m_data[sz];
	}

	T& expand(const T& fillValue = T())
	{
		GMuint sz = size();
		if (sz == capacity())
		{
			reserve(allocSize(size()));
		}
		m_size++;
#ifdef GM_USE_PLACEMENT_NEW
		new (&m_data[sz]) T(fillValue); //use the in-place new (not really allocating heap memory)
#endif
		return m_data[sz];
	}

	void push_back(const T& _Val)
	{
		GMuint sz = size();
		T __Val; // 重新分配空间的时候，如果_Val来自于自身，有可能会被销毁，这个时候应该拷贝一次
		if (sz == capacity())
		{
			__Val = _Val;
			reserve(allocSize(size()));
#ifdef GM_USE_PLACEMENT_NEW
			new (&m_data[m_size]) T(__Val);
#else
			m_data[size()] = __Val;
#endif //GM_USE_PLACEMENT_NEW
		}
		else
		{
#ifdef GM_USE_PLACEMENT_NEW
			new (&m_data[m_size]) T(_Val);
#else
			m_data[size()] = _Val;
#endif //GM_USE_PLACEMENT_NEW
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
			T*	s = (T*)allocate(_Count);
			GM_ASSERT(s);
			if (s == 0)
			{
				gm_error(_L("AlignedVector reserve out-of-memory\n"));
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
		T x = m_data[(lo + hi) / 2];

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
		char	temp[sizeof(T)];
		memcpy(temp, &m_data[index0], sizeof(T));
		memcpy(&m_data[index0], &m_data[index1], sizeof(T));
		memcpy(&m_data[index1], temp, sizeof(T));
#else
		T temp = m_data[index0];
		m_data[index0] = m_data[index1];
		m_data[index1] = temp;
#endif //GM_USE_PLACEMENT_NEW
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

	GMint findBinarySearch(const T& key) const
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


	GMint findLinearSearch(const T& key) const
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

	GMint findLinearSearch2(const T& key) const
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

	void remove(const T& key)
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
	AlignedAllocator<T, 16>	m_allocator;
	GMint m_size;
	GMint m_capacity;
	T* m_data;
	bool m_ownsMemory;
};

#endif

END_NS
#endif
