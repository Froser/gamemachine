#ifndef __SOUNDREADER_H__
#define __SOUNDREADER_H__
#include "common.h"
BEGIN_NS

#ifndef _WINDOWS
struct WAVEFORMATEX
{
	
};
#endif

struct WaveData
{
	GMbyte chID[4];
	DWORD dwSize;
	GMbyte* data;

	~WaveData() { if (data) delete[] data; }

	enum
	{
		HEADER_SIZE = 8,
	};

	static void newWaveData(OUT WaveData** d) { *d = new WaveData(); }

private:
	WaveData() : data(nullptr) {};
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
	virtual WaveData* getData() = 0;
};

struct GamePackageBuffer;
class Object;
struct ISoundReader
{
	virtual ~ISoundReader() {}
	virtual bool load(GamePackageBuffer& buffer, OUT ISoundFile** sf) = 0;
	virtual bool test(const GamePackageBuffer& buffer) = 0;
};

enum SoundType
{
	SoundType_AUTO,
	SoundType_Begin,
	SoundType_Wav = SoundType_Begin,
	SoundType_MP3,
	SoundType_End,
};

class SoundFile : public ISoundFile
{
public:
	typedef WAVEFORMATEX DataType;

public:
	SoundFile(const WAVEFORMATEX& fmt, AUTORELEASE WaveData* waveData);
	virtual ~SoundFile();

public:
	virtual WAVEFORMATEX* getWaveFormat() override;
	virtual WaveData* getData() override;

private:
	WAVEFORMATEX m_waveFormat;
	WaveData* m_waveData;
};

class SoundReader
{
public:
	static bool load(GamePackageBuffer& buffer, OUT ISoundFile** object);
	static bool load(GamePackageBuffer& buffer, SoundType type, OUT ISoundFile** object);
	static ISoundReader* getReader(SoundType type);

private:
	static SoundType test(const GamePackageBuffer& buffer);
};

END_NS
#endif
