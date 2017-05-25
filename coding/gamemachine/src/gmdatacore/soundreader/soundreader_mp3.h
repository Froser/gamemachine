#ifndef __SOUNDREADER_MP3_H__
#define __SOUNDREADER_MP3_H__
#include "common.h"
#include "soundreader.h"
#include "foundation/vector.h"
BEGIN_NS

struct SoundReader_MP3Private
{
};

class SoundReader_MP3 : public GMObject, public ISoundReader
{
	DECLARE_PRIVATE(SoundReader_MP3)

public:
	virtual bool load(GamePackageBuffer& buffer, OUT ISoundFile** sf) override;
	virtual bool test(const GamePackageBuffer& buffer) override;
};

END_NS
#endif
