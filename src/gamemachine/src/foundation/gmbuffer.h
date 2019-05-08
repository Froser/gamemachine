#ifndef __GMBUFFER_H__
#define __GMBUFFER_H__
#include <defines.h>
BEGIN_NS

class GM_EXPORT GMBuffer : public IDestroyObject
{
public:
	GMBuffer();
	GMBuffer(const GMBuffer&);
	GMBuffer(GMBuffer&&) GM_NOEXCEPT;
	GMBuffer(GMbyte* data, GMsize_t size, bool isOwned = true);
	~GMBuffer();

	GMBuffer& operator=(const GMBuffer&);
	GMBuffer& operator=(GMBuffer&&) GM_NOEXCEPT;

private:
	GMBuffer(GMBuffer& buf, GMsize_t offset);

public:
	static GMBuffer createBufferView(const GMBuffer& buf, GMsize_t offset);
	static GMBuffer createBufferView(GMbyte* data, GMsize_t size);

public:
	const GMbyte* getData() const;
	GMbyte* getData();
	GMsize_t getSize() const;
	bool isOwnedBuffer() const;
	void resize(GMsize_t, GMbyte* = nullptr);
	void swap(GMBuffer& rhs);
	void convertToStringBuffer();

private:
	void addRef();
	void releaseRef();

private:
	GMbyte* data;
	GMsize_t size;
	GMAtomic<GMuint32>* ref;
	bool isOwned;
};

END_NS
#endif