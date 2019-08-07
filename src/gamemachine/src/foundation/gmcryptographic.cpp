#include "stdafx.h"
#include "gmcryptographic.h"
#include "cryptographic/md5.h"

BEGIN_NS

void GMCryptographic::hash(const GMBuffer& in, Alogrithm method, GMBuffer& out)
{
	GM_ASSERT(method == MD5);

	out.resize(16);
	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, in.getData(), (GMulong) in.getSize());
	MD5_Final(out.getData(), &ctx);
}

END_NS