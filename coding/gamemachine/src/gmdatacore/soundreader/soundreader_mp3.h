#ifndef __SOUNDREADER_MP3_H__
#define __SOUNDREADER_MP3_H__
#include "common.h"
#include "soundreader.h"
BEGIN_NS

class SoundReader_MP3 : public ISoundReader
{
public:
	virtual bool load(GamePackageBuffer& buffer, OUT ISoundFile** sf) override;
	virtual bool test(const GamePackageBuffer& buffer) override;

private:
	bool decode(GamePackageBuffer& buffer);
};

END_NS
#endif