#ifndef __GMUNIBUFFER_H__
#define __GMUNIBUFFER_H__
#include <gmcommon.h>
BEGIN_NS

GM_INTERFACE(IUniProxy)
{
	virtual bool analyze() = 0;
	virtual bool CPtoX(GMint cp, bool trail, GMint* x) = 0;
	virtual bool XtoCP(GMint x, GMint* cp, bool* trail) = 0;
	virtual void getPriorItemPos(GMint cp, GMint* prior) = 0;
	virtual void getNextItemPos(GMint cp, GMint* prior) = 0;
};

GM_PRIVATE_OBJECT(GMUniBuffer)
{
	GMString buffer;
	IUniProxy* proxy;
};

class GMUniBuffer
{
	GM_DECLARE_PRIVATE(GMUniBuffer);

public:
	GMUniBuffer();
	~GMUniBuffer();

public:
	inline const GMString& getBuffer()
	{
		D(d);
		return d->buffer;
	}
};

struct GMUniProxyFactory
{
	static void createUniProxy(GMUniBuffer* buffer, OUT IUniProxy** proxy);
};

END_NS
#endif