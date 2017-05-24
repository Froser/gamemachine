enum EndiannessMode
{
	BSP_LITTLE_ENDIAN = 0,
	BSP_BIG_ENDIAN = 1
};

EndiannessMode getMachineEndianness()
{
	long int i = 1;
	const char *p = (const char *)&i;
	if (p[0] == 1)  // Lowest address contains the least significant byte
		return BSP_LITTLE_ENDIAN;
	else
		return BSP_BIG_ENDIAN;
}

int LittleLong(int l)
{
	static EndiannessMode mode = getMachineEndianness();
	if (mode == BSP_BIG_ENDIAN)
	{
		unsigned char b1, b2, b3, b4;

		b1 = l & 255;
		b2 = (l >> 8) & 255;
		b3 = (l >> 16) & 255;
		b4 = (l >> 24) & 255;

		return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
	}

	//little endian
	return l;
}

/*
=============
SwapBlock

If all values are 32 bits, this can be used to swap everything
=============
*/
void SwapBlock(int *block, int sizeOfBlock) {
	int		i;

	sizeOfBlock >>= 2;
	for (i = 0; i < sizeOfBlock; i++) {
		block[i] = LittleLong(block[i]);
	}
}

/*
=============
CopyLump
=============
*/
int CopyLump(BSPHeader* header, int lump, void *dest, int size)
{
	int		length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;

	if (length % size) {
		gm_error("LoadBSPFile: odd lump size");
	}

	memcpy(dest, (GMbyte *)header + ofs, length);

	return length / size;
}

char* copystring(const char *s)
{
	char *b;
	GMint len = strlen(s) + 1;
	b = (char*)malloc(len);
	strcpy_s(b, len, s);
	return b;
}

void stripTrailing(char *e) {
	char *s;

	s = e + strlen(e) - 1;
	while (s >= e && *s <= 32)
	{
		*s = 0;
		s--;
	}
}

std::string expandPath(const char *path)
{
	std::string strPath = Path::getCurrentPath();
	strPath.append(path);
	return strPath;
}

/*
================
Q_filelength
================
*/
int Q_filelength(FILE *f)
{
	int		pos;
	int		end;

	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	end = ftell(f);
	fseek(f, pos, SEEK_SET);

	return end;
}


void SafeRead(FILE *f, void *buffer, int count)
{
	if (fread(buffer, 1, count, f) != (size_t)count)
		gm_error("File read failure");
}

FILE *SafeOpenRead(const char *filename)
{
	FILE *f = nullptr;

	fopen_s(&f, filename, "rb");

	if (!f)
		gm_error("Error opening %s.", filename);

	return f;
}

/*
==============
LoadFile
==============
*/
int LoadFile(const char *filename, void **bufferptr)
{
	FILE	*f;
	int    length;
	void    *buffer;

	f = SafeOpenRead(filename);
	length = Q_filelength(f);
	buffer = malloc(length + 1);
	((char *)buffer)[length] = 0;
	SafeRead(f, buffer, length);
	fclose(f);

	*bufferptr = buffer;
	return length;
}
