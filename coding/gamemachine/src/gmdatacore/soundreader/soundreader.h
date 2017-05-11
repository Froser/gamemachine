#ifndef __SOUNDREADER_H__
#define __SOUNDREADER_H__
#include "common.h"
BEGIN_NS

struct GamePackageBuffer;
class Object;
struct ISoundReader
{
	virtual bool load(GamePackageBuffer& buffer, OUT Object** object) = 0;
	virtual bool test(const GamePackageBuffer& buffer) = 0;
};

enum SoundType
{
	SoundType_AUTO,
	SoundType_Begin,
	SoundType_Wav = SoundType_Begin,
	SoundType_End,
};

class SoundReader
{
public:
	static bool load(GamePackageBuffer& buffer, OUT Object** object);
	static bool load(GamePackageBuffer& buffer, SoundType type, OUT Object** object);
	static ISoundReader* getReader(SoundType type);

private:
	static SoundType test(const GamePackageBuffer& buffer);
};

END_NS
#endif