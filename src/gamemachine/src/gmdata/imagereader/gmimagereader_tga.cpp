#include "stdafx.h"
#include "gmimagereader_tga.h"
#include "foundation/utilities/tools.h"

namespace {

	/* Tell C++ that we have C types and declarations. */
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

/* Some macros to cope with non-ANSI C or C++ compilers.
 * (NOTE: This shouldn't be neccessary anymore nowadays)
 */
#undef __P
#if defined STDC_HEADERS || defined __cplusplus
# define __P(args) args
#else
# define __P(args) ()
#endif

 /* sections */
#define TGA_IMAGE_ID	0x01
#define TGA_IMAGE_INFO	0x02
#define TGA_IMAGE_DATA	0x04
#define TGA_COLOR_MAP	0x08
/* RLE */
#define TGA_RLE_ENCODE  0x10

/* color format */
#define TGA_RGB		0x20
#define TGA_BGR		0x40

/* orientation */
#define TGA_BOTTOM	0x0
#define TGA_TOP		0x1
#define	TGA_LEFT	0x0
#define	TGA_RIGHT	0x1

/* version info */
#define LIBTGA_VER_MAJOR  	1
#define LIBTGA_VER_MINOR  	0
#define LIBTGA_VER_PATCH	1
#define LIBTGA_VER_STRING	"1.0.1"

/* error codes */
	enum {
		TGA_OK = 0, 		/* success */
		TGA_ERROR,
		TGA_OOM,		/* out of memory */
		TGA_OPEN_FAIL,
		TGA_SEEK_FAIL,
		TGA_READ_FAIL,
		TGA_WRITE_FAIL,
		TGA_UNKNOWN_SUB_FORMAT  /* invalid bit depth */
	};

#define TGA_ERRORS 8  /* total number of error codes */

	/* text strings corresponding to the error codes */
	static char*
		tga_error_strings[] = {
			"Success",
			"Error",
			"Out of memory",
			"Failed to open file",
			"Seek failed",
			"Read failed",
			"Write failed",
			"Unknown sub-format"
	};


#if SIZEOF_UNSIGNED_INT == 4
	typedef unsigned int tuint32;
	typedef unsigned short tuint16;
#else
	typedef unsigned long tuint32;
	typedef unsigned int tuint16;
#endif

	typedef unsigned char tuint8;

	typedef tuint8  tbyte;
	typedef tuint16	tshort;
	typedef tuint32	tlong;

	typedef struct _TGAHeader TGAHeader;
	typedef struct _TGAData	  TGAData;
	typedef struct _TGA	  TGA;


	typedef void(*TGAErrorProc)(TGA*, int);


	/* TGA image header */
	struct _TGAHeader {
		tbyte	id_len;			/* image id length */
		tbyte	map_t;			/* color map type */
		tbyte	img_t;			/* image type */
		tshort	map_first;		/* index of first map entry */
		tshort	map_len;		/* number of entries in color map */
		tbyte	map_entry;		/* bit-depth of a cmap entry */
		tshort	x;				/* x-coordinate */
		tshort	y;				/* y-coordinate */
		tshort	width;			/* width of image */
		tshort	height;			/* height of image */
		tbyte	depth;			/* pixel-depth of image */
		tbyte   alpha;			/* alpha bits */
		tbyte	horz;			/* horizontal orientation */
		tbyte	vert;			/* vertical orientation */
	};

	/* TGA image data */
	struct _TGAData {
		tbyte	*img_id = 0;	/* image id */
		tbyte	*cmap = 0;	/* color map */
		tbyte	*img_data = 0;	/* image data */
		tuint32  flags;
	};

	/* TGA image handle */
	struct _TGA {
		GMMemoryStream*	ms = nullptr;	/* file stream */
		tlong			off;				/* current offset in file*/
		int				last;				/* last error code */
		TGAHeader		hdr;				/* image header */
		TGAErrorProc 	error = nullptr;	/* user-defined error proc */
	};

	__BEGIN_DECLS

		TGA* TGAOpen __P((char *name, char *mode));

	TGA* TGAOpenFd __P((GMMemoryStream *fd));

	int TGAReadHeader __P((TGA *tga));

	int TGAReadImageId __P((TGA *tga, tbyte **id));

	int TGAReadColorMap __P((TGA *tga, tbyte **cmap, tuint32 flags));

	size_t TGAReadScanlines __P((TGA *tga, tbyte *buf, size_t sln, size_t n,
		tuint32 flags));

	int TGAReadImage __P((TGA *tga, TGAData *data));

	char* TGAStrError __P((tuint8 code));

	tlong __TGASeek __P((TGA *tga, tlong off, int whence));

	void __TGAbgr2rgb __P((tbyte *data, size_t size, size_t bytes));

	__END_DECLS


#define TGA_HEADER_SIZE         18
#define TGA_CMAP_SIZE(tga)      ((tga)->hdr.map_len * (tga)->hdr.map_entry / 8)
#define TGA_CMAP_OFF(tga) 	(TGA_HEADER_SIZE + (tga)->hdr.id_len)
#define TGA_IMG_DATA_OFF(tga) 	(TGA_HEADER_SIZE + (tga)->hdr.id_len + TGA_CMAP_SIZE(tga))
#define TGA_IMG_DATA_SIZE(tga)	((tga)->hdr.width * (tga)->hdr.height * (tga)->hdr.depth / 8)
#define TGA_SCANLINE_SIZE(tga)	((tga)->hdr.width * (tga)->hdr.depth / 8)
#define TGA_CAN_SWAP(depth)     (depth == 24 || depth == 32)

#define TGA_IS_MAPPED(tga)      ((tga)->hdr.map_t == 1)
#define TGA_IS_ENCODED(tga)     ((tga)->hdr.img_t > 8 && (tga)->hdr.img_t < 12)

#define TGA_ERROR(tga, code) \
if((tga) && (tga)->error) (tga)->error(tga, code);\
gm_error(gm_dbg_wrap("Libtga:{0}"), TGAStrError(code));\
if(tga) (tga)->last = code\

		char*
		TGAStrError(tuint8 code)
	{
		if (code >= TGA_ERRORS) code = TGA_ERROR;
		return tga_error_strings[code];
	}

	tlong
		__TGASeek(TGA  *tga,
			tlong off,
			int   whence)
	{
		GMMemoryStream::SeekMode sm;
		switch (whence)
		{
		case SEEK_CUR:
			sm = GMMemoryStream::FromNow;
			break;
		case SEEK_SET:
			sm = GMMemoryStream::FromStart;
			break;
		default:
			sm = GMMemoryStream::FromStart;
			GM_ASSERT(false);
			break;
		}
		tga->ms->seek(off, sm);
		tga->off = tga->ms->tell();
		return tga->off;
	}

	void
		__TGAbgr2rgb(tbyte  *data,
			size_t  size,
			size_t  bytes)
	{
		size_t i;
		tbyte tmp;

		for (i = 0; i < size; i += bytes) {
			tmp = data[i];
			data[i] = data[i + 2];
			data[i + 2] = tmp;
		}
	}

	size_t
		TGARead(TGA    *tga,
			tbyte  *buf,
			size_t 	size,
			size_t 	n)
	{
		size_t read = tga->ms->read(buf, size * n);
		tga->off = tga->ms->tell();
		return read / size;
	}

	int
		TGAReadImage(TGA     *tga,
			TGAData *data)
	{
		if (!tga) return 0;

		if (TGAReadHeader(tga) != TGA_OK) {
			TGA_ERROR(tga, tga->last);
			return 0;
		}

		if ((data->flags & TGA_IMAGE_ID) && tga->hdr.id_len != 0) {
			if (TGAReadImageId(tga, &data->img_id) != TGA_OK) {
				data->flags &= ~TGA_IMAGE_ID;
				TGA_ERROR(tga, tga->last);
			}
		}
		else {
			data->flags &= ~TGA_IMAGE_ID;
		}

		if (data->flags & TGA_IMAGE_DATA) {
			if (TGA_IS_MAPPED(tga)) {
				if (!TGAReadColorMap(tga, &data->cmap, data->flags)) {
					data->flags &= ~TGA_COLOR_MAP;
					TGA_ERROR(tga, tga->last);
					return 0;
				}
				else {
					data->flags |= TGA_COLOR_MAP;
				}
			}

			data->img_data = (tbyte*)malloc(TGA_IMG_DATA_SIZE(tga));
			if (!data->img_data) {
				data->flags &= ~TGA_IMAGE_DATA;
				TGA_ERROR(tga, TGA_OOM);
				return 0;
			}

			if (TGAReadScanlines(tga, data->img_data, 0, tga->hdr.height, data->flags) != tga->hdr.height) {
				data->flags &= ~TGA_IMAGE_DATA;
				TGA_ERROR(tga, tga->last);
			}
		}
		tga->last = TGA_OK;
		return TGA_OK;
	}

	int
		TGAReadHeader(TGA *tga)
	{
		tbyte *tmp;

		if (!tga) return 0;

		__TGASeek(tga, 0, SEEK_SET);
		if (tga->off != 0) {
			TGA_ERROR(tga, TGA_SEEK_FAIL);
			return 0;
		}

		tmp = (tbyte*)malloc(TGA_HEADER_SIZE);
		if (!tmp) {
			TGA_ERROR(tga, TGA_OOM);
			return 0;
		}

		memset(tmp, 0, TGA_HEADER_SIZE);

		if (!TGARead(tga, tmp, TGA_HEADER_SIZE, 1)) {
			free(tmp);
			TGA_ERROR(tga, TGA_READ_FAIL);
			return 0;
		}

		tga->hdr.id_len = tmp[0];
		tga->hdr.map_t = tmp[1];
		tga->hdr.img_t = tmp[2];
		tga->hdr.map_first = tmp[3] + tmp[4] * 256;
		tga->hdr.map_len = tmp[5] + tmp[6] * 256;
		tga->hdr.map_entry = tmp[7];
		tga->hdr.x = tmp[8] + tmp[9] * 256;
		tga->hdr.y = tmp[10] + tmp[11] * 256;
		tga->hdr.width = tmp[12] + tmp[13] * 256;
		tga->hdr.height = tmp[14] + tmp[15] * 256;
		tga->hdr.depth = tmp[16];
		tga->hdr.alpha = tmp[17] & 0x0f;
		tga->hdr.horz = (tmp[17] & 0x10) ? TGA_TOP : TGA_BOTTOM;
		tga->hdr.vert = (tmp[17] & 0x20) ? TGA_RIGHT : TGA_LEFT;

		if (tga->hdr.map_t && tga->hdr.depth != 8) {
			TGA_ERROR(tga, TGA_UNKNOWN_SUB_FORMAT);
			free(tga);
			free(tmp);
			return 0;
		}

		if (tga->hdr.depth != 8 &&
			tga->hdr.depth != 15 &&
			tga->hdr.depth != 16 &&
			tga->hdr.depth != 24 &&
			tga->hdr.depth != 32)
		{
			TGA_ERROR(tga, TGA_UNKNOWN_SUB_FORMAT);
			free(tga);
			free(tmp);
			return 0;
		}

		free(tmp);
		tga->last = TGA_OK;
		return TGA_OK;
	}

	int
		TGAReadImageId(TGA    *tga,
			tbyte **buf)
	{
		if (!tga || tga->hdr.id_len == 0) return 0;

		__TGASeek(tga, TGA_HEADER_SIZE, SEEK_SET);
		if (tga->off != TGA_HEADER_SIZE) {
			TGA_ERROR(tga, TGA_SEEK_FAIL);
			return 0;
		}
		*buf = (tbyte*)malloc(tga->hdr.id_len);
		if (!buf) {
			TGA_ERROR(tga, TGA_OOM);
			return 0;
		}

		if (!TGARead(tga, *buf, tga->hdr.id_len, 1)) {
			free(buf);
			TGA_ERROR(tga, TGA_READ_FAIL);
			return 0;
		}

		tga->last = TGA_OK;
		return TGA_OK;
	}

	int
		TGAReadColorMap(TGA 	  *tga,
			tbyte   **buf,
			tuint32   flags)
	{
		tlong i, n, off, read, tmp;

		if (!tga) return 0;

		n = TGA_CMAP_SIZE(tga);
		if (n <= 0) return 0;

		off = TGA_CMAP_OFF(tga);
		if (tga->off != off) __TGASeek(tga, off, SEEK_SET);
		if (tga->off != off) {
			TGA_ERROR(tga, TGA_SEEK_FAIL);
			return 0;
		}

		*buf = (tbyte*)malloc(n);
		if (!buf) {
			TGA_ERROR(tga, TGA_OOM);
			return 0;
		}

		if ((read = TGARead(tga, *buf, n, 1)) != 1) {
			TGA_ERROR(tga, TGA_READ_FAIL);
			return 0;
		}

		if (TGA_CAN_SWAP(tga->hdr.map_entry) && (flags & TGA_RGB)) {
			__TGAbgr2rgb(*buf, TGA_CMAP_SIZE(tga), tga->hdr.map_entry / 8);
		}

		if (tga->hdr.map_entry == 15 || tga->hdr.map_entry == 16) {
			n = read + read / 2;
			*buf = (tbyte*)realloc(*buf, n);
			if (!(*buf)) {
				TGA_ERROR(tga, TGA_OOM);
				return 0;
			}

			for (i = read - 1; i >= 0; i -= 2) {
				tmp = *buf[i - 1] + *buf[i] * 255;
				*buf[n - 2] = (tmp >> 10) & 0x1F;
				*buf[n - 1] = (tmp >> 5) & 0x1F;
				*buf[n] = (tmp >> 5) & 0x1F;
				n -= 3;
			}
		}

		tga->last = TGA_OK;
		return read;
	}

	int
		TGAReadRLE(TGA   *tga,
			tbyte *buf)
	{
		int head;
		char sample[4];
		tbyte k, repeat = 0, direct = 0, bytes = tga->hdr.depth / 8;
		tshort x;
		tshort width = tga->hdr.width;
		GMMemoryStream *ms = tga->ms;

		if (!tga || !buf) return TGA_ERROR;

		for (x = 0; x < width; ++x) {
			if (repeat == 0 && direct == 0) {
				head = ms->get();
				if (head == EOF) return TGA_ERROR;
				if (head >= 128) {
					repeat = head - 127;
					if (ms->read(buf, bytes) < 1)
						return TGA_ERROR;
				}
				else {
					direct = head + 1;
				}
			}
			if (repeat > 0) {
				for (k = 0; k < bytes; ++k) buf[k] = sample[k];
				--repeat;
			}
			else {
				if (ms->read(buf, bytes) < 1) return TGA_ERROR;
				--direct;
			}
			buf += bytes;
		}

		tga->last = TGA_OK;
		return TGA_OK;
	}

	size_t
		TGAReadScanlines(TGA 	*tga,
			tbyte  *buf,
			size_t  sln,
			size_t  n,
			tuint32 flags)
	{
		tlong i, off;
		size_t sln_size, read, tmp;

		if (!tga || !buf) return 0;

		sln_size = TGA_SCANLINE_SIZE(tga);
		off = TGA_IMG_DATA_OFF(tga) + (sln * sln_size);

		if (tga->off != off) __TGASeek(tga, off, SEEK_SET);
		if (tga->off != off) {
			TGA_ERROR(tga, TGA_SEEK_FAIL);
			return 0;
		}

		if (TGA_IS_ENCODED(tga)) {
			for (read = 0; read <= n; ++read) {
				if (TGAReadRLE(tga, buf + ((sln + read) * sln_size)) !=
					TGA_OK) break;
			}
			tga->hdr.img_t -= 8;
		}
		else {
			read = TGARead(tga, buf, sln_size, n);
		}
		if (read != n) {
			TGA_ERROR(tga, TGA_READ_FAIL);
			return read;
		}

		if (TGA_CAN_SWAP(tga->hdr.depth) && (flags & TGA_RGB)) {
			__TGAbgr2rgb(buf + (sln_size * sln), sln_size * n,
				tga->hdr.depth / 8);
		}

		if (tga->hdr.depth == 15 || tga->hdr.depth == 16) {
			n = read + read / 2;
			buf = (tbyte*)realloc(buf, n);
			if (!buf) {
				TGA_ERROR(tga, TGA_OOM);
				return 0;
			}

			for (i = read - 1; i >= 0; i -= 2) {
				tmp = buf[i - 1] + buf[i] * 255;
				buf[n - 2] = (tmp >> 10) & 0x1F;
				buf[n - 1] = (tmp >> 5) & 0x1F;
				buf[n] = (tmp >> 5) & 0x1F;
				n -= 3;
			}
		}

		tga->last = TGA_OK;
		return read;
	}

	void copyTgaRgba(const TGA& tga, const GMbyte* source, GMbyte* dest)
	{
		// tga数据和我们所定义的图像坐标是颠倒的
		GMsize_t rowBytes = tga.hdr.width * GMImageReader::DefaultChannels;
		for (GMint i = tga.hdr.height - 1; i >= 0; --i)
		{
			GMsize_t destOffset = rowBytes * (tga.hdr.height - i - 1);
			GMsize_t srcOffset = rowBytes * i;
			memcpy_s(dest + destOffset, rowBytes, source + srcOffset, rowBytes);
		}
	}
}

bool GMImageReader_TGA::load(const GMbyte* data, GMsize_t size, OUT GMImage** image)
{
	GMImage* img = new GMImage();
	*image = img;
	GMImage::Data& imgData = img->getData();
	//init image data
	imgData.target = GMImageTarget::Texture2D;
	imgData.mipLevels = 1;
	imgData.type = GMImageDataType::UnsignedByte;

	TGA tga;
	GMMemoryStream ms(data, size);
	tga.ms = &ms;
	tga.last = TGA_OK;
	TGAData tgaData;
	tgaData.flags = TGA_IMAGE_ID | TGA_IMAGE_DATA | TGA_RGB;
	TGAReadImage(&tga, &tgaData);
	if (tga.last != TGA_OK)
	{
		gm_error(gm_dbg_wrap("Error in reading tga"));
		return false;
	}

	// 图像深度/8得到每个像素的字节数，这里认为1个通道就占一个字节，因此通道数即图像深度/8
	int channels = tga.hdr.depth / 8;
	imgData.mip[0].width = tga.hdr.width;
	imgData.mip[0].height = tga.hdr.height;
	imgData.format = GMImageFormat::RGBA;
	imgData.internalFormat = GMImageInternalFormat::RGBA8;
	imgData.channels = GMImageReader::DefaultChannels;

	GMuint bufferSize = tga.hdr.width * tga.hdr.height * GMImageReader::DefaultChannels;
	imgData.mip[0].data = new GMbyte[bufferSize];
	imgData.size = bufferSize;
	
	if (channels == 4)
	{
		copyTgaRgba(tga, tgaData.img_data, imgData.mip[0].data);
	}
	else if (channels == 3)
	{
		// 先构造一个TGA的RGBA数据
		GMbyte* tmpData = new GMbyte[bufferSize];
		GMsize_t ptr = 0;
		for (GMsize_t i = 0; i < tga.hdr.width * tga.hdr.height * channels; ++i, ++ptr)
		{
			*(tmpData + ptr) = *(tgaData.img_data + i);
			if ((i + 1) % 3 == 0)
				*(tmpData + ++ptr) = 0xFF;
		}
		GM_ASSERT(ptr == tga.hdr.width * tga.hdr.height * GMImageReader::DefaultChannels);

		// 再按照4通道的方法拷贝数据
		copyTgaRgba(tga, tmpData, imgData.mip[0].data);
	}
	else
	{
		gm_error(gm_dbg_wrap("GameMachine do not support non-rgb or non-rgba tga format."));
		GM_ASSERT(false);
		return false;
	}

	return true;
}

bool GMImageReader_TGA::test(const GMbyte* data)
{
	return
		data[3] == 0 ||
		data[3] == 1 ||
		data[3] == 2 ||
		data[3] == 3 ||
		data[3] == 9 ||
		data[3] == 10 ||
		data[3] == 11
	;
}