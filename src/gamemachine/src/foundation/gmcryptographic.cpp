#include "stdafx.h"
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "gmcryptographic.h"
#include <md5.h>

void GMCryptographic::hash(const GMBuffer& in, Alogrithm method, GMBuffer& out)
{
	GM_ASSERT(method == MD5);

	CryptoPP::Weak1::MD5 hash;
	out.size = CryptoPP::Weak1::MD5::DIGESTSIZE;
	out.buffer = new GMbyte[CryptoPP::Weak1::MD5::DIGESTSIZE];
	out.needRelease = true;
	hash.CalculateDigest(out.buffer, in.buffer, in.size);
}
