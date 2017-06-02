#ifndef __IMAGEREADER_TGA_H__
#define __IMAGEREADER_TGA_H__
#include "common.h"
#include <sstream>
#include "gmimagereader.h"
BEGIN_NS

#pragma pack(push,1)
struct TGA_Header {
	char idlength;
	char colormaptype;
	char datatypecode;
	short colormaporigin;
	short colormaplength;
	char colormapdepth;
	short x_origin;
	short y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
};
#pragma pack(pop)

struct TGAColor {
	unsigned char bgra[4];
	unsigned char bytespp;

	TGAColor() :  bytespp(1) 
	{
		for (int i=0; i<4; i++) 
			bgra[i] = 0;
	}

	TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A=255) : bytespp(4) {
		bgra[0] = B;
		bgra[1] = G;
		bgra[2] = R;
		bgra[3] = A;
	}

	TGAColor(unsigned char v) :  bytespp(1) {
		for (int i=0; i<4; i++) bgra[i] = 0;
		bgra[0] = v;
	}


	TGAColor(const unsigned char *p, unsigned char bpp) :  bytespp(bpp) {
		for (int i=0; i<(int)bpp; i++) {
			bgra[i] = p[i];
		}
		for (int i=bpp; i<4; i++) {
			bgra[i] = 0;
		}
	}

	unsigned char& operator[](const int i) { return bgra[i]; }

	TGAColor operator *(float intensity) const {
		TGAColor res = *this;
		intensity = (intensity>1.f?1.f:(intensity<0.f?0.f:intensity));
		for (int i=0; i<4; i++) res.bgra[i] = bgra[i]*intensity;
		return res;
	}
};

class MemoryStream;
class TGAImage
{
protected:
	GMbyte* d;
	int width;
	int height;
	int bytespp;
	size_t nbytes;

	bool load_rle_data(MemoryStream& in);
	bool unload_rle_data(std::ofstream &out) const;
public:
	enum Format {
		GRAYSCALE=1, RGB=3, RGBA=4
	};

	TGAImage();
	TGAImage(int w, int h, int bpp);
	TGAImage(const TGAImage &img);
	bool read_tga_file(const GMbyte* data, GMuint size);
	bool write_tga_file(const char *filename, bool rle=true) const;
	bool flip_horizontally();
	bool flip_vertically();
	bool scale(int w, int h);
	TGAColor get(int x, int y) const;
	
	bool set(int x, int y, TGAColor &c);
	bool set(int x, int y, const TGAColor &c);
	~TGAImage();
	TGAImage & operator =(const TGAImage &img);
	int get_width();
	int get_height();
	int get_bytespp();
	size_t get_nbytes();
	unsigned char *buffer();
	void clear();
};

class Image;
class GMImageReader_TGA : public IImageReader
{
public:
	virtual bool load(const GMbyte* data, GMuint size, OUT Image** img) override;
	virtual bool test(const GMbyte* data) override;

private:
	void writeDataToImage(TGAImage& tga, Image* img);
};

END_NS
#endif //__IMAGE_H__

