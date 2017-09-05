#ifndef __SOUNDREADER_H__
#define __SOUNDREADER_H__
#include <gmcommon.h>
BEGIN_NS

#ifndef _WINDOWS
struct WAVEFORMATEX
{
	
};
#endif

struct GMWaveData
{
	GMbyte chID[4];
	DWORD dwSize;
	GMbyte* data;

	~GMWaveData() { if (data) delete[] data; }

	enum
	{
		HEADER_SIZE = 8,
	};

	static void newWaveData(OUT GMWaveData** d) { *d = new GMWaveData(); }

private:
	GMWaveData() : data(nullptr) {};
};

struct PlayOptions
{
};

struct ISoundFile;
struct ISoundPlayer
{
	virtual ~ISoundPlayer() {};
	virtual void play() = 0;
	virtual void stop() = 0;
};

struct ISoundFile : public ISoundPlayer
{
	virtual ~ISoundFile() {}
	virtual WAVEFORMATEX* getWaveFormat() = 0;
	virtual GMWaveData* getData() = 0;
};

struct GMBuffer;
class GMModel;
struct ISoundReader
{
	virtual ~ISoundReader() {}
	virtual bool load(GMBuffer& buffer, OUT ISoundFile** sf) = 0;
	virtual bool test(const GMBuffer& buffer) = 0;
};

class GMSoundFile : public GMObject, public ISoundFile
{
public:
	typedef WAVEFORMATEX DataType;

public:
	GMSoundFile(const WAVEFORMATEX& fmt, AUTORELEASE GMWaveData* waveData);
	virtual ~GMSoundFile();

public:
	virtual WAVEFORMATEX* getWaveFormat() override;
	virtual GMWaveData* getData() override;

private:
	WAVEFORMATEX m_waveFormat;
	GMWaveData* m_waveData;
};

class GMSoundReader
{
public:
	enum SoundType
	{
		SoundType_AUTO,
		SoundType_Begin,
		SoundType_Wav = SoundType_Begin,
		SoundType_MP3,
		SoundType_End,
	};

	static bool load(GMBuffer& buffer, OUT ISoundFile** object);
	static bool load(GMBuffer& buffer, SoundType type, OUT ISoundFile** object);
	static ISoundReader* getReader(SoundType type);

private:
	static SoundType test(const GMBuffer& buffer);
};

END_NS
#endif
