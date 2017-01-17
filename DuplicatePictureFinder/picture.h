#pragma once

#include <assert.h>
#include <stdio.h>

enum
{
    E_ImageType_Unknown,
    E_ImageType_Bmp,
    E_ImageType_Png,
    E_ImageType_Jpg
};

struct ImageTypeUnknown
{

};

struct ImageTypeBmp
{
};

struct ImageTypeJpg
{
};

struct ImageTypePng
{
};

template <class T>
struct ImageTraits
{
    typedef typename T::ImageType ImageCategory;
};


struct ImageInfo
{
    typedef ImageTypeUnknown ImageType;
    int width;
    int height;
    int component;
    unsigned char *ppixels;

    ImageInfo() : ppixels(nullptr) { }
    ~ImageInfo()
    {
        if (ppixels)
            delete[] ppixels;
    }
};

struct ImageInfoBmp : public ImageInfo
{
    typedef ImageTypeBmp ImageType;
};

struct ImageInfoJpg : public ImageInfo
{
    typedef ImageTypeJpg ImageType;
};

struct ImageInfoPng : public ImageInfo
{
    typedef ImageTypePng ImageType;
};

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER
{
    unsigned short bfType;
    unsigned long  bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long  bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
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
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#pragma pack()


int GetImageType(FILE *infile);

bool GetImageInfo_Impl(FILE *infile, ImageInfoBmp *pinfo, ImageTypeBmp);
bool GetImageInfo_Impl(FILE *infile, ImageInfoJpg *pinfo, ImageTypeJpg);
bool GetImageInfo_Impl(FILE *infile, ImageInfoPng *pinfo, ImageTypePng);

bool GetImageRawData_Impl(FILE *infile, ImageInfo *pinfo, ImageTypeUnknown);
bool GetImageRawData_Impl(FILE *infile, ImageInfoBmp *pinfo, ImageTypeBmp);
bool GetImageRawData_Impl(FILE *infile, ImageInfoJpg *pinfo, ImageTypeJpg);
bool GetImageRawData_Impl(FILE *infile, ImageInfoPng *pinfo, ImageTypePng);

bool SaveToNewPicture_Impl(FILE *outfile, ImageInfo *pinfo, ImageTypeUnknown);
bool SaveToNewPicture_Impl(FILE *outfile, ImageInfoBmp *pinfo, ImageTypeBmp);
bool SaveToNewPicture_Impl(FILE *outfile, ImageInfoJpg *pinfo, ImageTypeJpg);
bool SaveToNewPicture_Impl(FILE *outfile, ImageInfoPng *pinfo, ImageTypePng);


////////////////////////////////////////////////////////////////////////////////

template <class T>
bool GetImageInfo(FILE *infile, T *pinfo)
{
    return GetImageInfo_Impl(infile, pinfo, ImageTraits<T>::ImageCategory());
}


////////////////////////////////////////////////////////////////////////////////


template <class T>
bool GetImageRawData(FILE *infile, T *pinfo)
{
    if (!pinfo)
        return false;

    return GetImageRawData_Impl(infile, pinfo, ImageTraits<T>::ImageCategory());
}


////////////////////////////////////////////////////////////////////////////////


template <class T>
bool SaveToNewPicture(FILE *outfile, T *pinfo)
{
    if (!pinfo)
        return false;

    return SaveToNewPicture_Impl(outfile, pinfo, ImageTraits<T>::ImageCategory());
}