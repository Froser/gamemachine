#ifndef __GMMAUDIOPLAYER_H__
#define __GMMAUDIOPLAYER_H__

#include <gmmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>

BEGIN_MEDIA_NS
class ALDeviceList;

GM_PRIVATE_CLASS(GMMAudioPlayer);
class GMMAudioPlayer : public gm::GMObject, public gm::IAudioPlayer
{
	GM_DECLARE_PRIVATE(GMMAudioPlayer)

public:
	GMMAudioPlayer();
	~GMMAudioPlayer();

public:
	ALDeviceList& getDevices();

public:
	virtual void createPlayerSource(gm::IAudioFile* f, OUT gm::IAudioSource** handle) override;

private:
	bool openDevice(gm::GMint32 idx);
	void shutdownDevice();
};

END_MEDIA_NS

#endif //__GMMAUDIOPLAYER_H__