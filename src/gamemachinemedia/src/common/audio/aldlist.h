#ifndef ALDEVICELIST_H
#define ALDEVICELIST_H

#pragma warning(disable: 4786)  //disable warning "identifier was truncated to '255' characters in the browser information"
#include <gmmcommon.h>
#include <vector>
#include <string>

using namespace std;

BEGIN_MEDIA_NS

typedef struct ALDEVICEINFO_
{
	string			strDeviceName;
	int				iMajorVersion = 0;
	int				iMinorVersion = 0;
	unsigned int	uiSourceCount = 0;
	vector<string>	pvstrExtensions;
	bool			bSelected = false;
} ALDEVICEINFO, *LPALDEVICEINFO;

class ALDeviceList
{
private:
	vector<ALDEVICEINFO> vDeviceInfo;
	int defaultDeviceIndex;
	int filterIndex;

public:
	ALDeviceList ();
	~ALDeviceList ();
	int GetNumDevices();
	char *GetDeviceName(int index);
	void GetDeviceVersion(int index, int *major, int *minor);
	unsigned int GetMaxNumSources(int index);
	bool IsExtensionSupported(int index, char *szExtName);
	int GetDefaultDevice();
	void FilterDevicesMinVer(int major, int minor);
	void FilterDevicesMaxVer(int major, int minor);
	void FilterDevicesExtension(char *szExtName);
	void ResetFilters();
	int GetFirstFilteredDevice();
	int GetNextFilteredDevice();

private:
	unsigned int GetMaxNumSources();
};

END_MEDIA_NS

#endif // ALDEVICELIST_H
