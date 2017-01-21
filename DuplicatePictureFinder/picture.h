#pragma once

#include <assert.h>
#include <stdio.h>

enum E_ImageType
{
    E_ImageType_Unknown,
    E_ImageType_Bmp,
    E_ImageType_Png,
    E_ImageType_Jpg
};

struct ImageInfo
{
    int width;
    int height;
    int component;

    // 像素点的颜色分量顺序按RGB(A)排列
    // 高度与bmp一样是倒向的
    unsigned char *ppixels;

    ImageInfo() : ppixels(nullptr) { }
    ~ImageInfo()
    {
        if (ppixels)
            delete[] ppixels;
    }
};

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    unsigned short bfType;
    unsigned long  bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long  bfOffBits;
} _BITMAPFILEHEADER;

typedef struct {
    unsigned long  biSize;
    long           biWidth;
    long           biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long  biCompression;
    unsigned long  biSizeImage;
    long           biXPelsPerMeter;
    long           biYPelsPerMeter;
    unsigned long  biClrUsed;
    unsigned long  biClrImportant;
} _BITMAPINFOHEADER;

#pragma pack(pop)


int GetImageType(FILE *infile);

bool GetImageInfo_Bmp_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageInfo_Jpg_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageInfo_Png_Impl(FILE *infile, ImageInfo *pinfo);

bool GetImageRawData_Bmp_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageRawData_Jpg_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageRawData_Png_Impl(FILE *infile, ImageInfo *pinfo);

bool SaveToNewPicture_Bmp_Impl(FILE *outfile, ImageInfo *pinfo);
bool SaveToNewPicture_Jpg_Impl(FILE *outfile, ImageInfo *pinfo);
bool SaveToNewPicture_Png_Impl(FILE *outfile, ImageInfo *pinfo);

bool GetImageInfo(FILE *infile, ImageInfo *pinfo, E_ImageType type);

bool GetImageRawData(FILE *infile, ImageInfo *pinfo, E_ImageType type);
bool GetImageRawData(const wchar_t *filename, ImageInfo *pinfo);
bool GetImageRawData(const char *filename, ImageInfo *pinfo);

bool SaveToNewPicture(FILE *outfile, ImageInfo *pinfo, E_ImageType type);
bool SaveToNewPicture(const wchar_t *filename, ImageInfo *pinfo, E_ImageType type);
bool SaveToNewPicture(const char *filename, ImageInfo *pinfo, E_ImageType type);

bool StretchPixels(const ImageInfo *in, ImageInfo *out);
bool StretchPixels_Shrink(const ImageInfo *in, ImageInfo *out);
bool StretchPixels_Expand(const ImageInfo *in, ImageInfo *out);

int OtsuThresholding(const int *histogram, int total);

bool CreateGray(const ImageInfo *in, ImageInfo *out);