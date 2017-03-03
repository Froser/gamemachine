/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

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

	memcpy(dest, (byte *)header + ofs, length);

	return length / size;
}

/*
=============
SwapBSPFile

Byte swaps all data in a bsp file.
=============
*/
void SwapBSPFile(BSPPrivate& d) {
	int i;

	// models	
	SwapBlock((int *)&d.models[0], d.nummodels * sizeof(d.models[0]));

	// shaders (don't swap the name)
	for (i = 0; i < d.numShaders; i++) {
		d.shaders[i].contentFlags = LittleLong(d.shaders[i].contentFlags);
		d.shaders[i].surfaceFlags = LittleLong(d.shaders[i].surfaceFlags);
	}

	// planes
	SwapBlock((int *)&d.planes[0], d.numplanes * sizeof(d.planes[0]));

	// nodes
	SwapBlock((int *)&d.nodes[0], d.numnodes * sizeof(d.nodes[0]));

	// leafs
	SwapBlock((int *)&d.leafs[0], d.numleafs * sizeof(d.leafs[0]));

	// leaffaces
	SwapBlock((int *)&d.leafsurfaces[0], d.numleafsurfaces * sizeof(d.leafsurfaces[0]));

	// leafbrushes
	SwapBlock((int *)&d.leafbrushes[0], d.numleafbrushes * sizeof(d.leafbrushes[0]));

	// brushes
	SwapBlock((int *)&d.brushes[0], d.numbrushes * sizeof(d.brushes[0]));

	// brushsides
	SwapBlock((int *)&d.brushsides[0], d.numbrushsides * sizeof(d.brushsides[0]));

	// vis
	((int *)&d.visBytes)[0] = LittleLong(((int *)&d.visBytes)[0]);
	((int *)&d.visBytes)[1] = LittleLong(((int *)&d.visBytes)[1]);

	// drawindexes
	SwapBlock((int *)&d.drawIndexes[0], d.numDrawIndexes * sizeof(d.drawIndexes[0]));

	// drawsurfs
	SwapBlock((int *)&d.drawSurfaces[0], d.numDrawSurfaces * sizeof(d.drawSurfaces[0]));
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