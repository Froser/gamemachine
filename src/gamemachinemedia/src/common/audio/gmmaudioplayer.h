#ifndef __GMMAUDIOPLAYER_H__
#define __GMMAUDIOPLAYER_H__

#include <gmmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>
#include "aldlist.h"

BEGIN_MEDIA_NS

GM_PRIVATE_OBJECT(GMMAudioPlayer)
{
	ALDeviceList devices;
};

class GMMAudioPlayer : public gm::GMObject, public gm::IAudioPlayer
{
	GM_DECLARE_PRIVATE(GMMAudioPlayer)

public:
	GMMAudioPlayer();
	~GMMAudioPlayer();

public:
	ALDeviceList& getDevices() { D(d); return d->devices; }

public:
	virtual void createPlayerSource(gm::IAudioFile* f, OUT gm::IAudioSource** handle) override;

private:
	bool openDevice(gm::GMint32 idx);
	void shutdownDevice();
};

END_MEDIA_NS

#endif //__GMMAUDIOPLAYER_H__