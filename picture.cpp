
#include <assert.h>
#include <iterator>
#include <string.h>
#include <stdio.h>
#include <vector>

#include "Utility.h"
#include "picture.h"
#include "include/libpng/png.h"
#include "include/libjpeg/jpeglib.h"


struct Jpg_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};


////////////////////////////////////////////////////////////////////////////////


void JpgErrorExitRoutine(j_common_ptr cinfo)
{
    (*cinfo->err->output_message) (cinfo);
    longjmp(((Jpg_error_mgr*)cinfo->err)->setjmp_buffer, 1);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


namespace Alisa
{
    class ImageImpl
    {
    public:
        ~ImageImpl() = default;

        bool Open(const string_t & filename);
        bool NewImage(ImageInfo info);
        bool SaveTo(const string_t & filename, E_ImageType type);
        void Clear();
        ImageInfo GetImageInfo() const;
        const std::vector<std::vector<Pixel>> & GetPixelsGroup() const;
        bool Blend(const ImageImpl *obj, int offsetX, int offsetY, E_ImageBlendMode mode);
        bool CopyPixelInLine(int dstLineOffset, int dstRowOffset, ImageImpl * srcObj, int srcLineOffset, int srcRowOffset, int cnt = -1);
        void ModifyPixels(std::function<void(int row, int col, Pixel &)> func);
        void WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const;
        bool StretchTo(ImageImpl *dst, int width, int height) const;
        int  OtsuThresholding() const;
        Image CreateGray() const;
        bool RemoveAlpha();
        bool AddAlpha();
        
        // ���/�Ƴ� lapha ͨ��

        // ico / gif / mpng / tiff / webp / ��ɫ��

    private:
        E_ImageType GetImageType(const string_t & filename);
        Pixel AlphaBlend(const Pixel & src, const Pixel & dst) const;
        Pixel SrcCopy(const Pixel & src, const Pixel & dst) const;
        bool BilinearInterpolation(ImageImpl *dst, int width, int height) const;
        bool LocalMeans(ImageImpl *dst, int width, int height) const;

    private:
        ImageInfo BaseInfo;
        std::vector<std::vector<Pixel>> Pixels;
        // Pixel ** Row;

    private:
        friend class ImageCodec;
    };

    class ImageCodec
    {
    public:
        static bool DecodeBmp(const string_t & filename,       ImageImpl *img);
        static bool EncodeBmp(const string_t & filename, const ImageImpl *img);

        static bool DecodePng(const string_t & filename,       ImageImpl *img);
        static bool EncodePng(const string_t & filename, const ImageImpl *img);

        static bool DecodeJpg(const string_t & filename,       ImageImpl *img);
        static bool EncodeJpg(const string_t & filename, const ImageImpl *img);
    };
}




////////////////////////////////////////////////////////////////////////////////////



Alisa::Image::Image()
{
    Impl = new ImageImpl;
}

Alisa::Image::Image(const Image & image)
{
}

Alisa::Image::Image(Image && image)
{
}

Alisa::Image::~Image()
{
    delete Impl;
    Impl = nullptr;
}

bool Alisa::Image::Open(const string_t & filename)
{
    return Impl->Open(filename);
}

bool Alisa::Image::NewImage(ImageInfo info)
{
    return Impl->NewImage(info);
}

bool Alisa::Image::SaveTo(const string_t & filename, E_ImageType type)
{
    return Impl->SaveTo(filename, type);
}

bool Alisa::Image::RemoveAlpha()
{
    return Impl->RemoveAlpha();
}

bool Alisa::Image::AddAlpha()
{
    return Impl->AddAlpha();
}

Alisa::ImageInfo Alisa::Image::GetImageInfo() const
{
    return Impl->GetImageInfo();
}

const std::vector<std::vector<Alisa::Pixel>>& Alisa::Image::GetPixelsGroup() const
{
    return Impl->GetPixelsGroup();
}

void Alisa::Image::Clear()
{
    return Impl->Clear();
}

bool Alisa::Image::Blend(const Image * image, int offsetX, int offsetY, E_ImageBlendMode mode)
{
    return Impl->Blend(image->Impl, offsetX, offsetY, mode);
}

bool Alisa::Image::CopyPixelInLine(int dstLineOffset, int dstRowOffset, Image * srcObj, int srcLineOffset, int srcRowOffset, int cnt)
{
    return Impl->CopyPixelInLine(dstLineOffset, dstRowOffset, srcObj->Impl, srcLineOffset, srcRowOffset, cnt);
}

void Alisa::Image::ModifyPixels(std::function<void(int row, int col, Pixel &)> func)
{
    Impl->ModifyPixels(func);
}

void Alisa::Image::WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const
{
    Impl->WalkPixels(func);
}

bool Alisa::Image::StretchTo(Image *dst, int width, int height) const
{
    return Impl->StretchTo(dst->Impl, width, height);
}

int Alisa::Image::OtsuThresholding() const
{
    return Impl->OtsuThresholding();
}

Alisa::Image Alisa::Image::CreateGray() const
{
    return Impl->CreateGray();
}



////////////////////////////////////////////////////////////////////////////////////



bool Alisa::ImageImpl::Open(const string_t & filename)
{
    auto type = GetImageType(filename);
    switch (type)
    {
    case E_ImageType_Bmp:
        return ImageCodec::DecodeBmp(filename, this);

    case E_ImageType_Jpg:
        return ImageCodec::DecodeJpg(filename, this);

    case E_ImageType_Png:
        return ImageCodec::DecodePng(filename, this);

    default:
        assert(0);
        break;
    }

    return false;
}

bool Alisa::ImageImpl::NewImage(ImageInfo info)
{
    BaseInfo = info;
    Pixels.resize(info.Height);
    for (auto & line : Pixels)
        line.resize(info.Width);
    return true;
}

bool Alisa::ImageImpl::SaveTo(const string_t & filename, E_ImageType type)
{
    switch (type)
    {
    case E_ImageType_Bmp:
        return ImageCodec::EncodeBmp(filename, this);

    case E_ImageType_Jpg:
        return ImageCodec::EncodeJpg(filename, this);

    case E_ImageType_Png:
        return ImageCodec::EncodePng(filename, this);

    default:
        assert(0);
        break;
    }

    return false;
}

void Alisa::ImageImpl::Clear()
{
    BaseInfo.Reset();
    Pixels.clear();
}

Alisa::ImageInfo Alisa::ImageImpl::GetImageInfo() const
{
    return BaseInfo;
}

const std::vector<std::vector<Alisa::Pixel>> & Alisa::ImageImpl::GetPixelsGroup() const
{
    return Pixels;
}

Alisa::Pixel Alisa::ImageImpl::AlphaBlend(const Pixel & src, const Pixel & dst) const
{
    float srcA = (float)src.A / 0xff;
    float dstA = (float)dst.A / 0xff;

    Pixel blend;
    blend.R = src.R * srcA + dst.R * (1 - srcA);
    blend.G = src.G * srcA + dst.G * (1 - srcA);
    blend.B = src.B * srcA + dst.B * (1 - srcA);
    blend.A = (srcA + dstA * (1 - srcA)) * 0xff;

    return blend;
}

Alisa::Pixel Alisa::ImageImpl::SrcCopy(const Pixel & src, const Pixel & dst) const
{
    return src;
}

bool Alisa::ImageImpl::Blend(const ImageImpl * obj, int offsetX, int offsetY, E_ImageBlendMode mode)
{
    if (offsetX < 0 || offsetX > BaseInfo.Width - obj->BaseInfo.Width ||
        offsetY < 0 || offsetY > BaseInfo.Height - obj->BaseInfo.Height)
    {
        assert(0);
        return false;
    }

    for (size_t h = 0; h < obj->BaseInfo.Height; ++h)
    {
        for (size_t w = 0; w < obj->BaseInfo.Width; ++w)
        {
            const auto & srcPixel = obj->Pixels[h][w];
            if (srcPixel.A > 0)
            {
                auto & dstPixel = Pixels[offsetY + h][offsetX + w];

                switch (mode)
                {
                case E_AlphaBlend:
                    dstPixel = AlphaBlend(srcPixel, dstPixel);
                    break;
                case E_SrcOver:
                    dstPixel = SrcCopy(srcPixel, dstPixel);
                    break;
                default:
                    assert(0);
                    return false;
                }
            }
        }
    }

    return true;
}

bool Alisa::ImageImpl::CopyPixelInLine(int dstLineOffset, int dstRowOffset, ImageImpl * srcObj, int srcLineOffset, int srcRowOffset, int cnt)
{
    if (cnt == -1)
    {
        cnt = srcObj->BaseInfo.Width - srcRowOffset;
    }

    if (cnt <= 0)
    {
        assert(0);
        return false;
    }

    if (dstLineOffset >= BaseInfo.Height || dstRowOffset + cnt > BaseInfo.Width)
    {
        assert(0);
        return false;
    }

    if (!srcObj || srcLineOffset >= srcObj->BaseInfo.Height || srcRowOffset + cnt > srcObj->BaseInfo.Width)
    {
        assert(0);
        return false;
    }


    std::copy(
        stdext::make_checked_array_iterator(srcObj->Pixels[srcLineOffset].data(), srcObj->Pixels[srcLineOffset].size(), srcRowOffset),
        stdext::make_checked_array_iterator(srcObj->Pixels[srcLineOffset].data(), srcObj->Pixels[srcLineOffset].size(), srcRowOffset + cnt),
        stdext::make_checked_array_iterator(Pixels[dstLineOffset].data(), Pixels[dstLineOffset].size(), dstRowOffset)
    );
    return true;
}

void Alisa::ImageImpl::ModifyPixels(std::function<void(int row, int col, Pixel &)> func)
{
    for (size_t row = 0; row < Pixels.size(); ++row)
    {
        for (size_t col = 0; col < Pixels[row].size(); ++col)
        {
            func(row, col, Pixels[row][col]);
        }
    }
}

void Alisa::ImageImpl::WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const
{
    for (size_t row = 0; row < Pixels.size(); ++row)
    {
        for (size_t col = 0; col < Pixels[row].size(); ++col)
        {
            func(row, col, Pixels[row][col]);
        }
    }
}

bool Alisa::ImageImpl::LocalMeans(ImageImpl *dst, int width, int height) const
{
    // �ֲ���ֵ��С

    const float k1 = (float)BaseInfo.Width / width;      //  > 1
    const float k2 = (float)BaseInfo.Height / height;    //  > 1
    assert(k1 >= 1 && k2 >= 1);

    const int block = (int)k1 * (int)k2;

    dst->BaseInfo = BaseInfo;
    dst->BaseInfo.Width = width;
    dst->BaseInfo.Height = height;
    dst->Pixels.resize(height);
    for (int i = 0; i < height; ++i)
        dst->Pixels[i].resize(width);

    if (BaseInfo.Component == 3 || BaseInfo.Component == 4)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int r = 0, g = 0, b = 0, a = 0;
                for (int j = 0; j < (int)k2; ++j)
                {
                    for (int i = 0; i < (int)k1; ++i)
                    {
                        const Pixel & p = Pixels[y * k2 + j][x * k1 + i];
                        r += p.R;
                        g += p.G;
                        b += p.B;
                        a += p.A;
                    }
                }
                Pixel & target = dst->Pixels[y][x];

                assert(r / block < 0x100);
                assert(g / block < 0x100);
                assert(b / block < 0x100);
                assert(a / block < 0x100);

                target.R = r / block;
                target.G = g / block;
                target.B = b / block;
                target.A = a / block;
            }
        }
    }
    else
    {
        assert(0);
        return false;
    }

    return true;
}

bool Alisa::ImageImpl::BilinearInterpolation(ImageImpl *dst, int width, int height) const
{
    // ˫���Բ�ֵ

    const float k1 = (float)width / BaseInfo.Width;      //  > 1
    const float k2 = (float)height / BaseInfo.Height;    //  > 1
    //assert(k1 >= 1 && k2 >= 1);

    dst->BaseInfo = BaseInfo;
    dst->BaseInfo.Width = width;
    dst->BaseInfo.Height = height;
    dst->Pixels.resize(height);
    for (int y = 0; y < height; ++y)
        dst->Pixels[y].resize(width);

    for (int y = 0; y < BaseInfo.Height; ++y)
    {
        for (int x = 0; x < BaseInfo.Width; ++x)
        {
            int top = y * k2;
            int bottom = (y + 1) * k2;
            int left = x * k1;
            int right = (x + 1) * k1;

            int block = (bottom - top) * (right - left);

            const Pixel & p1 = Pixels[y][x];
            const Pixel & p2 = x < BaseInfo.Width - 1 ? Pixels[y][x + 1] : p1;
            const Pixel & p3 = Pixels[(y < BaseInfo.Height - 1 ? y + 1 : BaseInfo.Height - 1)][x];
            const Pixel & p4 = x < BaseInfo.Width - 1 ? Pixels[(y < BaseInfo.Height - 1 ? y + 1 : BaseInfo.Height - 1)][x + 1] : p3;

            for (int j = top; j < bottom; ++j)
            {
                for (int i = left; i < right; ++i)
                {
                    Pixel & targetPixel = dst->Pixels[j][i];

                    float f1 = (float)(bottom - j) * (right - i) / block;       // (1-u)(1-v)
                    float f2 = (float)(bottom - j) * (i - left) / block;        // u(1-v)
                    float f3 = (float)(j - top) * (right - i) / block;          // (1-u)v
                    float f4 = (float)(j - top) * (i - left) / block;           // uv

                    int r = f1 * p1.R + f2 * p2.R + f3 * p3.R + f4 * p4.R;
                    int g = f1 * p1.G + f2 * p2.G + f3 * p3.G + f4 * p4.G;
                    int b = f1 * p1.B + f2 * p2.B + f3 * p3.B + f4 * p4.B;
                    int a = f1 * p1.A + f2 * p2.A + f3 * p3.A + f4 * p4.A;

                    assert(r < 0x100);
                    assert(g < 0x100);
                    assert(b < 0x100);

                    targetPixel.R = r;
                    targetPixel.G = g;
                    targetPixel.B = b;
                    targetPixel.A = a;
                }
            }
        }
    }

    return true;
}

bool Alisa::ImageImpl::StretchTo(ImageImpl *dst, int width, int height) const
{
    if (BaseInfo.Height <= 0 || BaseInfo.Width <= 0 || width <= 0 || height <= 0)
    {
        assert(0);
        return false;
    }

    if (width < BaseInfo.Width && height <= BaseInfo.Height)
        return LocalMeans(dst, width, height);
    else
        return BilinearInterpolation(dst, width, height);
}

int Alisa::ImageImpl::OtsuThresholding() const
{
    assert(0);
    return 0;
}

Alisa::Image Alisa::ImageImpl::CreateGray() const
{
    assert(0);
    return Image();
}

bool Alisa::ImageImpl::RemoveAlpha()
{
    if (BaseInfo.Component != PixelType_RGBA)
        return true;

    BaseInfo.Component = PixelType_RGB;
    for (size_t i = 0; i < Pixels.size(); ++i)
    {
        for (size_t k = 0; k < Pixels[i].size(); ++k)
        {
            Pixels[i][k].A = 0xff;
        }
    }
    return true;
}

bool Alisa::ImageImpl::AddAlpha()
{
    if (BaseInfo.Component == PixelType_RGBA)
        return true;

    assert(BaseInfo.Component == PixelType_RGB);
    if (BaseInfo.Component != PixelType_RGB)
        return false;

    BaseInfo.Component = PixelType_RGBA;
    for (size_t i = 0; i < Pixels.size(); ++i)
    {
        for (size_t k = 0; k < Pixels[i].size(); ++k)
        {
            Pixels[i][k].A = 0xff;
        }
    }

    return true;
}

Alisa::E_ImageType Alisa::ImageImpl::GetImageType(const string_t & filename)
{
    const unsigned char png_magic[] = { 0x89, 0x50, 0x4e, 0x47 };
    const unsigned char jpg_magic[] = { 0xff, 0xd8 };
    const unsigned char bmp_magic[] = { 0x42, 0x4d };


    FILE *infile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&infile, filename.c_str(), L"rb");
#else
    errno_t err = fopen_s(&infile, filename.c_str(), "rb");
#endif
    if (err)
    {
        assert(0);
        return E_ImageType_Unknown;
    }

    unsigned char buf[4];
    fread_s(buf, sizeof(buf), sizeof(buf), 1, infile);
    fclose(infile);

    E_ImageType type = E_ImageType_Unknown;

    switch (buf[0])
    {
    case 0x89:
        if (!memcmp(buf, png_magic, sizeof(png_magic)))
            type = E_ImageType_Png;
        break;

    case 0xff:
        if (jpg_magic[1] == buf[1])
            type = E_ImageType_Jpg;
        break;

    case 0x42:
        if (bmp_magic[1] == buf[1])
            type = E_ImageType_Bmp;
        break;
    }

    return type;
}



////////////////////////////////////////////////////////////////////////////////////



bool Alisa::ImageCodec::DecodeBmp(const string_t & filename, ImageImpl * img)
{
    assert(!filename.empty());
    assert(img);
    if (filename.empty() || !img)
        return false;

    img->BaseInfo.Width = img->BaseInfo.Height = img->BaseInfo.Component = 0;
    img->BaseInfo.FrameCount = 1;

    FILE *infile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&infile, filename.c_str(), L"rb");
#else
    errno_t err = fopen_s(&infile, filename.c_str(), "rb");
#endif
    if (err)
        return false;

    rewind(infile);

    _BITMAPFILEHEADER bfh;
    fread_s(&bfh, sizeof(_BITMAPFILEHEADER), sizeof(_BITMAPFILEHEADER), 1, infile);
    _BITMAPINFOHEADER bih;
    fread_s(&bih, sizeof(_BITMAPINFOHEADER), sizeof(_BITMAPINFOHEADER), 1, infile);

    if (bfh.bfType != 'MB')     // С����
        return false;

    if (bih.biBitCount < 8)
        return false;

    assert(bih.biBitCount == 24 || bih.biBitCount == 32);   // ��ɫ����˵

    img->BaseInfo.Height = abs(bih.biHeight);
    img->BaseInfo.Width = bih.biWidth;
    img->BaseInfo.Component = bih.biBitCount >> 3;
    int row_stride = bih.biWidth * (bih.biBitCount >> 3);
    int row_expand = (row_stride + 3) & ~3;             // bmp����4�ֽڶ���

    fseek(infile, 0, SEEK_END);
    int bytesize = ftell(infile) - bfh.bfOffBits;       // bfh.bfSize �ֶο���ס
    assert(bytesize >= img->BaseInfo.Height * row_stride);
    unsigned char *ppixels = new unsigned char[bytesize];

    fseek(infile, bfh.bfOffBits, SEEK_SET);
    for (int i = 0; i < img->BaseInfo.Height; ++i)
    {
        long dummy;
        // ��ȡʱ���Ե�ÿ�����ļ����ֽ�
        // ͬʱ���ݸ߶��Ƿ�Ϊ������ͼ����з�ת
        if (bih.biHeight > 0)
            fread_s(ppixels + i * row_stride, row_stride, 1, row_stride, infile);
        else
            fread_s(ppixels + (img->BaseInfo.Height - 1 - i) * row_stride, row_stride, 1, row_stride, infile);
        fread_s(&dummy, sizeof(long), 1, row_expand - row_stride, infile);

    }
    fclose(infile);
    infile = NULL;

    // �ڴ������ص����ɫ����˳����RGB(A)��bmp�ļ��е�˳����BGR(A)
    unsigned char *ptr = ppixels;
    for (int i = 0; i < img->BaseInfo.Height * img->BaseInfo.Width; ++i)
    {
        // ��B��R�ķ����Ի�
        unsigned char tmp;
        tmp = ptr[0]; ptr[0] = ptr[2]; ptr[2] = tmp;
        ptr += img->BaseInfo.Component;
    }


    img->Pixels.resize(img->BaseInfo.Height);
    for (size_t i = 0; i < img->Pixels.size(); ++i)
        img->Pixels[i].resize(img->BaseInfo.Width);

    // ������ת������˳���Ϊ�����½ǵ����Ͻ�
    for (int i = 0; i < img->BaseInfo.Height; ++i)
    {
        for (int k = 0; k < img->BaseInfo.Width; ++k)
        {
            Pixel & p = img->Pixels[img->BaseInfo.Height - 1 - i][k];
            unsigned char *ptr = ppixels + i * row_stride + k * img->BaseInfo.Component;
            p.R = *ptr;
            p.G = *(ptr + 1);
            p.B = *(ptr + 2);
            p.A = img->BaseInfo.Component == PixelType_RGBA ? *(ptr + 3) : 0xff;
        }
    }

    delete[] ppixels;

    return true;
}

bool Alisa::ImageCodec::EncodeBmp(const string_t & filename, const ImageImpl * img)
{
    const char zero_fill[8] = { 0 };

    int row_stride = img->BaseInfo.Width * img->BaseInfo.Component;
    int aligned_width = (row_stride + 3) & ~3;          // bmp ÿ����Ҫ4�ֽڶ���

    _BITMAPFILEHEADER bfh = { 0 };
    bfh.bfType = 'MB';  // С��
    bfh.bfSize = img->BaseInfo.Height * aligned_width +
        sizeof(_BITMAPFILEHEADER) + sizeof(_BITMAPINFOHEADER);
    bfh.bfOffBits = sizeof(_BITMAPFILEHEADER) + sizeof(_BITMAPINFOHEADER);

    _BITMAPINFOHEADER bih = { 0 };
    bih.biSize = sizeof(_BITMAPINFOHEADER);
    bih.biWidth = img->BaseInfo.Width;
    bih.biHeight = img->BaseInfo.Height;
    bih.biPlanes = 1;
    bih.biBitCount = img->BaseInfo.Component << 3;
    bih.biSizeImage = bfh.bfSize - bfh.bfOffBits;

    FILE *outfile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&outfile, filename.c_str(), L"wb");
#else
    errno_t err = fopen_s(&outfile, filename.c_str(), "wb");
#endif
    if (err)
        return false;

    fwrite(&bfh, sizeof(_BITMAPFILEHEADER), 1, outfile);
    fwrite(&bih, sizeof(_BITMAPINFOHEADER), 1, outfile);

    // �ڴ������ص����ɫ����˳����RGB(A)��bmp�ļ��е�˳����BGR(A)
    unsigned char *buffer = new unsigned char[row_stride];
    for (int i = img->BaseInfo.Height - 1; i >= 0; --i)
    {
        const auto & row = img->Pixels[i];

        if (img->BaseInfo.Component == PixelType_RGB)
        {
            for (int k = 0; k < img->BaseInfo.Width; ++k)
            {
                buffer[k * 3]     = row[k].B;
                buffer[k * 3 + 1] = row[k].G;
                buffer[k * 3 + 2] = row[k].R;
            }
        }
        else if (img->BaseInfo.Component == PixelType_RGBA)
        {
            for (int k = 0; k < img->BaseInfo.Width; ++k)
            {
                buffer[k * 4]     = row[k].B;
                buffer[k * 4 + 1] = row[k].G;
                buffer[k * 4 + 2] = row[k].R;
                buffer[k * 4 + 3] = row[k].A;
            }
        }
        else
        {
            assert(0);
            delete[] buffer;
            fclose(outfile);
            return false;
        }
        fwrite(buffer, 1, row_stride, outfile);
        fwrite(zero_fill, 1, aligned_width - row_stride, outfile);
    }

    fclose(outfile);

    delete[] buffer;
    return true;
}

bool Alisa::ImageCodec::DecodePng(const string_t & filename, ImageImpl * img)
{
    unsigned char* pPixels = nullptr;
    unsigned char** lines = nullptr;

    png_structp png_ptr;     //libpng�Ľṹ��
    png_infop   info_ptr;    //libpng����Ϣ

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        assert(0);
        return false;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        assert(0);
        return false;
    }

    int iRetVal = setjmp(png_jmpbuf(png_ptr));
    if (iRetVal)
    {
        fprintf(stderr, "�����룺%d\n", iRetVal);
        Utility::SafeDeleteArray(&pPixels);
        Utility::SafeDeleteArray(&lines);
        assert(0);
        return false;
    }

    assert(png_ptr && info_ptr);

    FILE *infile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&infile, filename.c_str(), L"rb");
#else
    errno_t err = fopen_s(&infile, filename.c_str(), "rb");
#endif
    if (err)
        return false;


    //
    // ��libpng���ļ���
    //
    png_init_io(png_ptr, infile);
    png_read_info(png_ptr, info_ptr);

    //
    // ��ȡ�ļ�ͷ��Ϣ
    //
    int bit_depth, color_type;
    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr,
        &width, &height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    //
    // ����ɫ��ʽ��ȡΪRGBA
    //

    int pixel_byte = color_type == PNG_COLOR_TYPE_RGB ? 3 : 4;

    //Ҫ��ת��������ɫ��RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    //Ҫ��λ���ǿ��8bit
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth<8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    //Ҫ��λ���ǿ��8bit
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    //�Ҷȱ���ת����RGB
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    //
    // �������ػ�����
    //
    pPixels = new unsigned char[width * height * pixel_byte];
    lines = new unsigned char*[height * sizeof(unsigned char*)]; //��ָ��

    png_int_32 h = height - 1;
    png_int_32 i = 0;
    while (h >= 0)//�������ȡ����Ϊλͼ�ǵ׵�����
    {
        lines[i] = &pPixels[h * width * pixel_byte];
        --h;
        ++i;
    }

    //
    // ��ȡ����
    //
    png_read_image(png_ptr, (png_bytepp)lines);

    img->Clear();
    img->BaseInfo.Height = height;
    assert(img->BaseInfo.Height > 0);
    img->BaseInfo.Width = width;
    img->BaseInfo.Component = pixel_byte;
    

    assert((int)&((Pixel*)0)->R == 0 && (int)&((Pixel*)0)->G == 1 && (int)&((Pixel*)0)->B == 2 && (int)&((Pixel*)0)->A == 3);

    //
    // png ͼ���ǵ��õģ���ȡ���ڴ���䵹ת���� Pixels[0] ��Ӧ�߼��ϵĵ�һ��
    // �� lines[0] = �ļ��е����һ�� = ��Ӧ�߼���/��ʾ��(��Ļ����ϵ)�ĵ�һ�� = Pixels[0]
    //
    img->Pixels.resize(height);
    for (size_t i = 0; i < img->Pixels.size(); ++i)
    {
        img->Pixels[i].resize(width);
        for (size_t k = 0; k < img->Pixels[i].size(); ++k)
        {
            Pixel & p = img->Pixels[i][k];
            p.R = *(lines[i] + k * pixel_byte);
            p.G = *(lines[i] + k * pixel_byte + 1);
            p.B = *(lines[i] + k * pixel_byte + 2);
            p.A = pixel_byte == PixelType_RGBA ? *(lines[i] + k * pixel_byte + 3) : 0xff;
        }
    }


    //
    // �ͷ���Դ
    //
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(infile);
    delete[] lines;
    delete[] pPixels;
    return true;
}

bool Alisa::ImageCodec::EncodePng(const string_t & filename, const ImageImpl * img)
{
    png_structp png_ptr;     //libpng�Ľṹ��
    png_infop   info_ptr;    //libpng����Ϣ

    unsigned char** lines = nullptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        goto Error;
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, NULL);
        goto Error;
    }

    if (int iRetVal = setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "�����룺%d\n", iRetVal);
        goto Error;
    }

    assert(png_ptr && info_ptr);

    FILE *outfile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&outfile, filename.c_str(), L"wb");
#else
    errno_t err = fopen_s(&outfile, filename.c_str(), "wb");
#endif
    if (err)
        return false;

    png_init_io(png_ptr, outfile);

    //
    //����PNG�ļ�ͷ
    //
    png_set_IHDR(
        png_ptr,
        info_ptr,
        img->BaseInfo.Width,
        img->BaseInfo.Height,
        8,                              //��ɫ���,
        img->BaseInfo.Component == PixelType_RGBA ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,//��ɫ����
        PNG_INTERLACE_NONE,             //����������: PNG_INTERLACE_ADAM7
        PNG_COMPRESSION_TYPE_DEFAULT,   //ѹ����ʽ
        PNG_FILTER_TYPE_DEFAULT         //ʲô����? Ĭ���� PNG_FILTER_TYPE_DEFAULT
    );
    //���ô����Ϣ
    png_set_packing(png_ptr);
    //д���ļ�ͷ
    png_write_info(png_ptr, info_ptr);

    lines = new unsigned char*[img->BaseInfo.Height * sizeof(unsigned char*)]; //��ָ��

    //
    // ͬ DecodePng, ����ת�ú󱣴�
    //
    if (img->BaseInfo.Component == PixelType_RGBA)
    {
        assert((int)&((Pixel*)0)->R == 0 && (int)&((Pixel*)0)->G == 1 && (int)&((Pixel*)0)->B == 2 && (int)&((Pixel*)0)->A == 3);

        for (png_int_32 i = 0; i < img->BaseInfo.Height; ++i)
        {
            lines[i] = new unsigned char[img->BaseInfo.Width * img->BaseInfo.Component];
            memcpy_s(lines[i], img->BaseInfo.Width * img->BaseInfo.Component, img->Pixels[i].data(), img->BaseInfo.Width * sizeof(Pixel));
        }
    }
    else
    {
        for (png_int_32 i = 0; i < img->BaseInfo.Height; ++i)
        {
            lines[i] = new unsigned char[img->BaseInfo.Width * img->BaseInfo.Component];
            for (int w = 0; w < img->BaseInfo.Width; ++w)
            {
                lines[i][w * img->BaseInfo.Component]     = img->Pixels[i][w].R;
                lines[i][w * img->BaseInfo.Component + 1] = img->Pixels[i][w].G;
                lines[i][w * img->BaseInfo.Component + 2] = img->Pixels[i][w].B;
            }
        }
    }

    //
    // д������
    //
    png_write_image(png_ptr, (png_bytepp)lines);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(outfile);
    for (int i = 0; i < img->BaseInfo.Height; ++i)
        delete[] lines[i];
    delete[] lines;

    return true;

Error:
    if (lines)
    {
        for (int i = 0; i < img->BaseInfo.Height; ++i)
            delete[] lines[i];
        delete[] lines;
    }
    assert(0);
    return false;
}

bool Alisa::ImageCodec::DecodeJpg(const string_t & filename, ImageImpl * img)
{
    struct jpeg_decompress_struct cinfo;
    struct Jpg_error_mgr jerr;
    JSAMPARRAY row_arr = nullptr;

    img->Clear();

    FILE *infile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&infile, filename.c_str(), L"rb");
#else
    errno_t err = fopen_s(&infile, filename.c_str(), "rb");
#endif
    if (err)
        return false;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = JpgErrorExitRoutine;

    if (setjmp(jerr.setjmp_buffer))
    {
        fprintf(stderr, "jpg��ʧ��\r\n");
        jpeg_destroy_decompress(&cinfo);
        if (row_arr)
            delete[] row_arr;
        assert(0);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);

    //assert(JCS_RGB == cinfo.out_color_space);
    if (JCS_RGB == cinfo.out_color_space)
    {
        img->BaseInfo.Height = cinfo.image_height;
        assert(img->BaseInfo.Height > 0);
        img->BaseInfo.Width = cinfo.image_width;
        img->BaseInfo.Component = cinfo.num_components;
        assert(cinfo.num_components == PixelType_RGB);

        int row_stride = cinfo.image_width * cinfo.num_components;
        auto ppixels = new unsigned char[row_stride * cinfo.image_height];

        jpeg_start_decompress(&cinfo);

        // jpg����˳�������ʾ˳��
        row_arr = new JSAMPROW[cinfo.image_height];
        for (JDIMENSION i = 0; i < cinfo.image_height; ++i)
            row_arr[i] = (JSAMPROW)(ppixels + (i) * row_stride);
        while (cinfo.output_scanline < cinfo.output_height)
            (void)jpeg_read_scanlines(&cinfo, &row_arr[cinfo.output_scanline], 1);

        jpeg_finish_decompress(&cinfo);

        img->Pixels.resize(cinfo.image_height);
        for (size_t r = 0; r < img->Pixels.size(); ++r)
        {
            img->Pixels[r].resize(cinfo.image_width);
            for (size_t w = 0; w < img->Pixels[r].size(); ++w)
            {
                auto & p = img->Pixels[r][w];
                p.R = ppixels[r * row_stride + w * cinfo.num_components];
                p.G = ppixels[r * row_stride + w * cinfo.num_components + 1];
                p.B = ppixels[r * row_stride + w * cinfo.num_components + 2];
                // p.A = 0xff;
            }
        }

        delete[] row_arr;
        delete[] ppixels;
    }
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return JCS_RGB == cinfo.out_color_space;
}

bool Alisa::ImageCodec::EncodeJpg(const string_t & filename, const ImageImpl * img)
{
    struct jpeg_compress_struct cinfo;
    struct Jpg_error_mgr jerr;
    unsigned char *translate = nullptr;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = JpgErrorExitRoutine;

    jpeg_create_compress(&cinfo);

    if (setjmp(jerr.setjmp_buffer))
    {
        fprintf(stderr, "jpg�洢ʧ��\r\n");
        jpeg_destroy_compress(&cinfo);
        if (translate)
            delete[] translate;
        assert(0);
        return false;
    }


    FILE *outfile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&outfile, filename.c_str(), L"wb");
#else
    errno_t err = fopen_s(&outfile, filename.c_str(), "wb");
#endif
    if (err)
        return false;


    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = img->BaseInfo.Width;
    cinfo.image_height = img->BaseInfo.Height;
    cinfo.in_color_space = JCS_RGB;
    cinfo.input_components = PixelType_RGB;     // RGB

    // alpha blend
    // ����˳��ΪRGBA
    translate = new unsigned char[cinfo.image_width * cinfo.image_height * cinfo.input_components];
    unsigned char *pdst = translate;
    unsigned char *pdst_end = translate + cinfo.image_width * cinfo.image_height * cinfo.input_components;
    for (size_t h = 0; h < cinfo.image_height; ++h)
    {
        for (size_t w = 0; w < cinfo.image_width; ++w)
        {
            const auto & p = img->Pixels[h][w];
            pdst[0] = p.R * p.A / 255 + 255 - p.A;
            pdst[1] = p.G * p.A / 255 + 255 - p.A;
            pdst[2] = p.B * p.A / 255 + 255 - p.A;
            pdst += 3;
            assert(pdst <= pdst_end);
        }
    }
    assert(pdst == pdst_end);


    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);

    jpeg_start_compress(&cinfo, TRUE);
    int row_stride = cinfo.image_width * cinfo.input_components;

    // jpg����˳�������ʾ˳��
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &translate[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    if (translate)
        delete[] translate;

    return true;
}
