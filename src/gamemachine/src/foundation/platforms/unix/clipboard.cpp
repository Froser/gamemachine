#include "stdafx.h"
#include "defines.h"
#include "foundation/utilities/tools.h"
#include "foundation/assert.h"

namespace
{
	struct GMClipboardData
	{
		Array<GMBuffer, static_cast<GMsize_t>(GMClipboardMIME::EndOfEnum)> data;
	} s_clipboardData;
}

void GMClipboard::setData(GMClipboardMIME mime, const GMBuffer& buffer)
{
	s_clipboardData.data[static_cast<GMsize_t>(mime)] = buffer;
}

GMBuffer GMClipboard::getData(GMClipboardMIME mime)
{
	return s_clipboardData.data[static_cast<GMsize_t>(mime)];
}