#include "stdafx.h"
#include "defines.h"
#include "foundation/utilities/tools.h"
#include "foundation/assert.h"

namespace
{
	UINT toWindowsClipboardFormat(GMClipboardMIME mime)
	{
		switch (mime)
		{
		case GMClipboardMIME::Text:
			return CF_TEXT;
		case GMClipboardMIME::Bitmap:
			return CF_BITMAP;
		case GMClipboardMIME::Riff:
			return CF_RIFF;
		case GMClipboardMIME::Wave:
			return CF_WAVE;
		case GMClipboardMIME::UnicodeText:
			return CF_UNICODETEXT;
		default:
			GM_ASSERT(false);
			return CF_UNICODETEXT;
		}
	}
}

void GMClipboard::setData(GMClipboardMIME mime, const GMBuffer& buffer)
{
	EmptyClipboard();

	if (OpenClipboard(NULL))
	{
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, buffer.getSize());
		if (handle)
		{
			GMbyte* ptr = static_cast<GMbyte*>(GlobalLock(handle));
			if (ptr)
			{
				memcpy_s(ptr, buffer.getSize(), buffer.getData(), buffer.getSize());
				GlobalUnlock(handle);
			}
			SetClipboardData(toWindowsClipboardFormat(mime), handle);
		}
		CloseClipboard();

		if (handle)
			GlobalFree(handle);
	}
}

GMBuffer GMClipboard::getData(GMClipboardMIME mime)
{
	GMBuffer buffer;
	if (OpenClipboard(NULL))
	{
		HGLOBAL handle = GetClipboardData(toWindowsClipboardFormat(mime));
		if (handle)
		{
			SIZE_T sz = GlobalSize(handle);
			buffer.resize(sz);
			memcpy_s(buffer.getData(), sz, static_cast<GMbyte*>(GlobalLock(handle)), sz);

			if (mime == GMClipboardMIME::Text || mime == GMClipboardMIME::UnicodeText)
				buffer.convertToStringBuffer();

			GlobalUnlock(handle);
		}
		CloseClipboard();
	}
	return buffer;
}