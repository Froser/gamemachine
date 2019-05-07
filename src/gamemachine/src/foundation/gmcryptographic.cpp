#include "stdafx.h"
#include "gmcryptographic.h"
#include <md5.h>

void GMCryptographic::hash(const GMBuffer& in, Alogrithm method, GMBuffer& out)
{
	GM_ASSERT(method == MD5);

	CryptoPP::MD5 hash;
	out.size = CryptoPP::MD5::DIGESTSIZE;
	out.buffer = new GMbyte[CryptoPP::MD5::DIGESTSIZE];
	out.needRelease = true;
	hash.CalculateDigest(out.buffer, in.buffer, in.size);
}
