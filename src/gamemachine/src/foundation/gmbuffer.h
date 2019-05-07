#ifndef __GMBUFFER_H__
#define __GMBUFFER_H__
#include <gmcommon.h>
BEGIN_NS

// 缓存类，用于存储缓存数据
class GM_EXPORT GMBuffer : public IDestroyObject
{
public:
	GMBuffer() = default;
	~GMBuffer();

	GMBuffer(const GMBuffer& rhs);
	GMBuffer(GMBuffer&& rhs) GM_NOEXCEPT;
	GMBuffer& operator =(GMBuffer&& rhs) GM_NOEXCEPT;
	GMBuffer& operator =(const GMBuffer& rhs);

public:
	void convertToStringBuffer();
	void convertToStringBufferW();
	void swap(GMBuffer& rhs);

public:
	GMbyte* buffer = nullptr;
	GMsize_t size = 0;
	bool needRelease = false; // 表示是否需要手动释放
};

class GM_EXPORT GMBufferView : public GMBuffer
{
public:
	GMBufferView(const GMBuffer&, GMsize_t offset);
	~GMBufferView();
};

END_NS
#endif