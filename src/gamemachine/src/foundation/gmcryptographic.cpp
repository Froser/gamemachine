#include "stdafx.h"
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "gmcryptographic.h"
#include <md5.h>

BEGIN_NS

void GMCryptographic::hash(const GMBuffer& in, Alogrithm method, GMBuffer& out)
{
	GM_ASSERT(method == MD5);

	CryptoPP::Weak1::MD5 hash;
	out.resize(CryptoPP::Weak1::MD5::DIGESTSIZE);
	hash.CalculateDigest(out.getData(), in.getData(), in.getSize());
}

END_NS