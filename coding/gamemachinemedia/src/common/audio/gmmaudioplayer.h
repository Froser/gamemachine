#ifndef __GMMAUDIOPLAYER_H__
#define __GMMAUDIOPLAYER_H__

#include <gmmcommon.h>
#include <gamemachine.h>
#include "aldlist.h"

BEGIN_MEDIA_NS

GM_PRIVATE_OBJECT(GMMAudioPlayer)
{
	ALDeviceList devices;
};

class GMMAudioPlayer : public gm::IAudioPlayer
{
	DECLARE_PRIVATE(GMMAudioPlayer)

public:
	GMMAudioPlayer();
	~GMMAudioPlayer();

public:
	ALDeviceList& getDevices() { D(d); return d->devices; }

public:
	virtual void play() override {}
	virtual void stop() override {}

private:
	bool openDevice(gm::GMint idx);
	void shutdownDevice();
};

END_MEDIA_NS

#endif //__GMMAUDIOPLAYER_H__