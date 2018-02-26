
#include <assert.h>
#include <iterator>
#include <string.h>
#include <stdio.h>
#include <vector>

#include "Utility.h"
#include "picture.h"
#include "libpng/png.h"
#include "libjpeg/jpeglib.h"


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


////////////////////////////////////////////////////////////////////////////////


#if 0
int GetImageType(FILE *infile)
{
    const unsigned char png_magic[] = { 0x89, 0x50, 0x4e, 0x47 };
    const unsigned char jpg_magic[] = { 0xff, 0xd8 };
    const unsigned char bmp_magic[] = { 0x42, 0x4d };

    rewind(infile);

    unsigned char buf[4];
    fread_s(buf, sizeof(buf), sizeof(buf), 1, infile);

    int type = E_ImageType_Unknown;

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


////////////////////////////////////////////////////////////////////////////////


bool GetImageInfo_Bmp_Impl(FILE *infile, ImageInfo *pinfo)
{
    if (!infile || !pinfo)
        return false;
    pinfo->width = pinfo->height = pinfo->component = 0;

    rewind(infile);

    _BITMAPFILEHEADER bfh;
    fread_s(&bfh, sizeof(_BITMAPFILEHEADER), sizeof(_BITMAPFILEHEADER), 1, infile);
    _BITMAPINFOHEADER bih;
    fread_s(&bih, sizeof(_BITMAPINFOHEADER), sizeof(_BITMAPINFOHEADER), 1, infile);

    if (bfh.bfType != 'BM')
        return false;

    if (bih.biBitCount < 8)
        return false;

    assert(bih.biBitCount = 24 || bih.biBitCount == 32);   // 调色板再说

    pinfo->height = bih.biHeight;
    pinfo->width = bih.biWidth;
    pinfo->component = bih.biBitCount >> 3;

    return true;
}

bool GetImageInfo_Jpg_Impl(FILE *infile, ImageInfo *pinfo)
{
    if (!infile || !pinfo)
        return false;
    pinfo->width = pinfo->height = pinfo->component = 0;

    rewind(infile);

    struct jpeg_decompress_struct cinfo;
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);

    assert(JCS_RGB == cinfo.out_color_space);
    pinfo->height = cinfo.image_height;
    pinfo->width = cinfo.image_width;
    pinfo->component = cinfo.num_components;

    jpeg_destroy_decompress(&cinfo);
    return true;
}

bool GetImageInfo_Png_Impl(FILE *infile, ImageInfo *pinfo)
{
    if (!infile || !pinfo)
        return false;
    pinfo->width = pinfo->height = pinfo->component = 0;

    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

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
        fprintf(stderr, "错误码：%d\n", iRetVal);
        assert(0);
        return false;
    }

    assert(png_ptr && info_ptr);
    assert(infile);
    rewind(infile);

    //
    // 绑定libpng和文件流
    //
    png_init_io(png_ptr, infile);
    png_read_info(png_ptr, info_ptr);

    //
    // 获取文件头信息
    //
    int bit_depth, color_type;
    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr,
        &width, &height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return true;
}


////////////////////////////////////////////////////////////////////////////////


bool GetImageRawData_Bmp_Impl(FILE *infile, ImageInfo *pinfo)
{
    if (!infile || !pinfo)
        return false;
    pinfo->width = pinfo->height = pinfo->component = 0;

    rewind(infile);

    _BITMAPFILEHEADER bfh;
    fread_s(&bfh, sizeof(_BITMAPFILEHEADER), sizeof(_BITMAPFILEHEADER), 1, infile);
    _BITMAPINFOHEADER bih;
    fread_s(&bih, sizeof(_BITMAPINFOHEADER), sizeof(_BITMAPINFOHEADER), 1, infile);

    if (bfh.bfType != 'MB')     // 小端序
        return false;

    if (bih.biBitCount < 8)
        return false;

    assert(bih.biBitCount == 24 || bih.biBitCount == 32);   // 调色板再说

    pinfo->height = bih.biHeight;
    pinfo->width = bih.biWidth;
    pinfo->component = bih.biBitCount >> 3;
    int row_stride = bih.biWidth * (bih.biBitCount >> 3);
    int row_expand = (row_stride + 3) & ~3;             // bmp的行4字节对齐

    fseek(infile, 0, SEEK_END);    
    int bytesize = ftell(infile) - bfh.bfOffBits;       // bfh.bfSize 字段靠不住
    assert(bytesize >= bih.biHeight * row_stride);
    pinfo->ppixels = new unsigned char[bytesize];

    fseek(infile, bfh.bfOffBits, SEEK_SET);
    for (int i=0; i<bih.biHeight; ++i)
    {
        long dummy;
        // 读取时忽略掉每行填充的几个字节
        fread_s(pinfo->ppixels + i * row_stride, row_stride, 1, row_stride, infile);
        fread_s(&dummy, sizeof(long), 1, row_expand - row_stride, infile);

    }

    // 内存中像素点的颜色分量顺序是RGB(A)，bmp文件中的顺序是BGR(A)
    unsigned char *ptr = pinfo->ppixels;
    for (int i = 0; i < pinfo->height * pinfo->width; ++i)
    {
        // 将B和R的分量对换
        unsigned char tmp;
        tmp = ptr[0]; ptr[0] = ptr[2]; ptr[2] = tmp;
        ptr += pinfo->component;
    }

    return true;
}

bool GetImageRawData_Jpg_Impl(FILE *infile, ImageInfo *pinfo)
{
    struct jpeg_decompress_struct cinfo;
    struct Jpg_error_mgr jerr;
    JSAMPARRAY row_arr = nullptr;

    if (!infile || !pinfo)
        return false;
    pinfo->width = pinfo->height = pinfo->component = 0;

    rewind(infile);

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = JpgErrorExitRoutine;

    if (setjmp(jerr.setjmp_buffer))
    {
        fprintf(stderr, "jpg打开失败\r\n");
        jpeg_destroy_decompress(&cinfo);
        if (row_arr)
            delete[] row_arr;
        if (pinfo->ppixels)
            delete[] pinfo->ppixels, pinfo->ppixels = nullptr;
        assert(0);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);

    //assert(JCS_RGB == cinfo.out_color_space);
    if (JCS_RGB == cinfo.out_color_space)
    {
        pinfo->height = cinfo.image_height;
        pinfo->width = cinfo.image_width;
        pinfo->component = cinfo.num_components;
        assert(cinfo.num_components == 3);

        int row_stride = cinfo.image_width * cinfo.num_components;
        pinfo->ppixels = new unsigned char[row_stride * pinfo->height];

        jpeg_start_decompress(&cinfo);

        // 按bmp/png把行像素倒置
        row_arr = new JSAMPROW[pinfo->height];
        for (int i = 0; i < pinfo->height; ++i)
            row_arr[i] = (JSAMPROW)(pinfo->ppixels + (pinfo->height - i - 1) * row_stride);
        while (cinfo.output_scanline < cinfo.output_height)
            (void)jpeg_read_scanlines(&cinfo, &row_arr[cinfo.output_scanline], 1);
        delete[] row_arr;

        jpeg_finish_decompress(&cinfo);
    }
    jpeg_destroy_decompress(&cinfo);
    return JCS_RGB == cinfo.out_color_space;
}

bool GetImageRawData_Png_Impl(FILE *infile, ImageInfo *pinfo)
{
    pinfo->width = pinfo->height = pinfo->component = 0;
    pinfo->ppixels = nullptr;

    unsigned char* pPixels = nullptr;
    unsigned char** lines = nullptr;

    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

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
        fprintf(stderr, "错误码：%d\n", iRetVal);
        if (pPixels)
            delete[] pPixels, pinfo->ppixels = nullptr;
        if (lines)
            delete[] lines;
        assert(0);
        return false;
    }

    assert(png_ptr && info_ptr);
    assert(infile);
    //rewind(infile);
    fseek(infile, 0, SEEK_SET);

    //
    // 绑定libpng和文件流
    //
    png_init_io(png_ptr, infile);
    png_read_info(png_ptr, info_ptr);

    //
    // 获取文件头信息
    //
    int bit_depth, color_type;
    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr,
        &width, &height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    //
    // 按颜色格式读取为RGBA
    //

    int pixel_byte = color_type == PNG_COLOR_TYPE_RGB ? 3 : 4;

    //要求转换索引颜色到RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    //要求位深度强制8bit
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth<8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    //要求位深度强制8bit
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    //灰度必须转换成RGB
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    //
    // 分配像素缓冲区
    //
    pPixels = new unsigned char[width * height * pixel_byte];
    lines = new unsigned char*[height * sizeof(unsigned char*)]; //列指针

    png_int_32 h = height - 1;
    png_int_32 i = 0;
    while (h >= 0)//逆行序读取，因为位图是底到上型
    {
        lines[i] = &pPixels[h * width * pixel_byte];
        --h;
        ++i;
    }

    //
    // 读取像素
    //
    png_read_image(png_ptr, (png_bytepp)lines);

    pinfo->height = height;
    pinfo->width = width;
    pinfo->component = pixel_byte;
    pinfo->ppixels = pPixels;


    //
    // 释放资源
    //
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    delete[] lines;

    return true;
}


////////////////////////////////////////////////////////////////////////////////


bool SaveToNewPicture_Bmp_Impl(FILE *outfile, ImageInfo *pinfo)
{
    const char zero_fill[8] = { 0 };

    int row_stride = pinfo->width * pinfo->component;
    int aligned_width = (row_stride + 3) & ~3;          // bmp 每行需要4字节对齐

    _BITMAPFILEHEADER bfh = { 0 };
    bfh.bfType = 'MB';  // 小端
    bfh.bfSize = pinfo->height * aligned_width +
        sizeof(_BITMAPFILEHEADER) + sizeof(_BITMAPINFOHEADER);
    bfh.bfOffBits = sizeof(_BITMAPFILEHEADER) + sizeof(_BITMAPINFOHEADER);

    _BITMAPINFOHEADER bih = { 0 };
    bih.biSize = sizeof(_BITMAPINFOHEADER);
    bih.biWidth = pinfo->width;
    bih.biHeight = pinfo->height;
    bih.biPlanes = 1;
    bih.biBitCount = pinfo->component << 3;
    bih.biSizeImage = bfh.bfSize - bfh.bfOffBits;

    fwrite(&bfh, sizeof(_BITMAPFILEHEADER), 1, outfile);
    fwrite(&bih, sizeof(_BITMAPINFOHEADER), 1, outfile);

    // 内存中像素点的颜色分量顺序是RGB(A)，bmp文件中的顺序是BGR(A)
    unsigned char *buffer = new unsigned char[row_stride];
    for (int i = 0; i < pinfo->height; ++i)
    {
        const unsigned char *cur_row = pinfo->ppixels + i * row_stride;

        if (pinfo->component == 3)
            for (int k = 0; k < pinfo->width; ++k)
            {
                buffer[k * 3]     = cur_row[k * 3 + 2];
                buffer[k * 3 + 1] = cur_row[k * 3 + 1];
                buffer[k * 3 + 2] = cur_row[k * 3];
            }

        else if (pinfo->component == 4)
            for (int k = 0; k < pinfo->width; ++k)
            {
                buffer[k * 4]     = cur_row[k * 4 + 2];
                buffer[k * 4 + 1] = cur_row[k * 4 + 1];
                buffer[k * 4 + 2] = cur_row[k * 4];
                buffer[k * 4 + 3] = cur_row[k * 4 + 3];
            }

        else
        {
            assert(0);
            delete[] buffer;
            return false;
        }
        fwrite(buffer, 1, row_stride, outfile);
        fwrite(zero_fill, 1, aligned_width - row_stride, outfile);
    }

    delete[] buffer;
    return true;
}

bool SaveToNewPicture_Jpg_Impl(FILE *outfile, ImageInfo *pinfo)
{
    struct jpeg_compress_struct cinfo;
    struct Jpg_error_mgr jerr;
    unsigned char *translate = nullptr;
    
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = JpgErrorExitRoutine;

    jpeg_create_compress(&cinfo);

    if (setjmp(jerr.setjmp_buffer))
    {
        fprintf(stderr, "jpg存储失败\r\n");
        jpeg_destroy_compress(&cinfo);
        if (translate)
            delete[] translate;
        assert(0);
        return false;
    }

    assert(outfile);
    rewind(outfile);

    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = pinfo->width;
    cinfo.image_height = pinfo->height;
    cinfo.in_color_space = JCS_RGB;
    unsigned char *src = pinfo->ppixels;

    if (pinfo->component == 4)
    {
        // alpha blend
        // 像素顺序为RGBA
        translate = new unsigned char[pinfo->width * pinfo->height * 3];
        unsigned char *pdst = translate, *psrc = pinfo->ppixels;
        for (int i = 0; i < pinfo->width * pinfo->height; ++i)
        {
            pdst[0] = psrc[0] * psrc[3] / 255 + 255 - psrc[3];
            pdst[1] = psrc[1] * psrc[3] / 255 + 255 - psrc[3];
            pdst[2] = psrc[2] * psrc[3] / 255 + 255 - psrc[3];
            psrc += 4;
            pdst += 3;
        }
        cinfo.input_components = 3;
        src = translate;
    }
    else
        cinfo.input_components = pinfo->component;


    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);

    jpeg_start_compress(&cinfo, TRUE);
    int row_stride = pinfo->width * cinfo.input_components;

    // 储存的时候倒着来
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &src[(cinfo.image_height - cinfo.next_scanline - 1) * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    if (translate)
        delete[] translate;

    return true;
}

bool SaveToNewPicture_Png_Impl(FILE *outfile, ImageInfo *pinfo)
{
    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

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
        fprintf(stderr, "错误码：%d\n", iRetVal);
        goto Error;
    }

    assert(png_ptr && info_ptr);
    assert(outfile);
    rewind(outfile);

    png_init_io(png_ptr, outfile);

    //
    //设置PNG文件头
    //
    png_set_IHDR(
        png_ptr,
        info_ptr,
        pinfo->width,
        pinfo->height,
        8,                              //颜色深度,
        pinfo->component == 4 ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,//颜色类型
        PNG_INTERLACE_NONE,             //不交错。交错: PNG_INTERLACE_ADAM7
        PNG_COMPRESSION_TYPE_DEFAULT,   //压缩方式
        PNG_FILTER_TYPE_DEFAULT         //什么过滤? 默认填 PNG_FILTER_TYPE_DEFAULT
    );
    //设置打包信息
    png_set_packing(png_ptr);
    //写入文件头
    png_write_info(png_ptr, info_ptr);

    lines = new unsigned char*[pinfo->height * sizeof(unsigned char*)]; //列指针

    png_int_32 i = 0, h = pinfo->height - 1;
    while (h >= 0)//逆行序读取，因为位图是底到上型
    {
        lines[i] = &pinfo->ppixels[h * pinfo->width * pinfo->component];
        ++i, --h;
    }

    //
    // 写入像素
    //
    png_write_image(png_ptr, (png_bytepp)lines);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    delete[] lines;

    return true;

Error:
    if (lines)
        delete[] lines;
    assert(0);
    return false;
}


////////////////////////////////////////////////////////////////////////////////


bool GetImageInfo(FILE *infile, ImageInfo *pinfo, E_ImageType type)
{
    switch (type)
    {
    case E_ImageType_Bmp:
        return GetImageInfo_Bmp_Impl(infile, pinfo);

    case E_ImageType_Jpg:
        return GetImageInfo_Jpg_Impl(infile, pinfo);

    case E_ImageType_Png:
        return GetImageInfo_Png_Impl(infile, pinfo);

    default:
        return false;
    }
}


////////////////////////////////////////////////////////////////////////////////


bool GetImageRawData(FILE *infile, ImageInfo *pinfo, E_ImageType type)
{
    switch (type)
    {
    case E_ImageType_Bmp:
        return GetImageRawData_Bmp_Impl(infile, pinfo);

    case E_ImageType_Jpg:
        return GetImageRawData_Jpg_Impl(infile, pinfo);

    case E_ImageType_Png:
        return GetImageRawData_Png_Impl(infile, pinfo);

    default:
        return false;
    }
}


bool GetImageRawData(const wchar_t *filename, ImageInfo *pinfo)
{
    errno_t err;
    FILE *infile;
    err = _wfopen_s(&infile, filename, L"rb");
    if (err)
        return false;

    bool ret = GetImageRawData(infile, pinfo, (E_ImageType)GetImageType(infile));

    fclose(infile);
    return ret;
}


bool GetImageRawData(const char *filename, ImageInfo *pinfo)
{
    errno_t err;
    FILE *infile;
    err = fopen_s(&infile, filename, "rb");
    if (err)
        return false;

    bool ret = GetImageRawData(infile, pinfo, (E_ImageType)GetImageType(infile));

    fclose(infile);
    return ret;
}


////////////////////////////////////////////////////////////////////////////////


bool SaveToNewPicture(FILE *outfile, ImageInfo *pinfo, E_ImageType type)
{
    switch (type)
    {
    case E_ImageType_Bmp:
        return SaveToNewPicture_Bmp_Impl(outfile, pinfo);

    case E_ImageType_Jpg:
        return SaveToNewPicture_Jpg_Impl(outfile, pinfo);

    case E_ImageType_Png:
        return SaveToNewPicture_Png_Impl(outfile, pinfo);

    default:
        return false;
    }
}


bool SaveToNewPicture(const wchar_t *filename, ImageInfo *pinfo, E_ImageType type)
{
    errno_t err;
    FILE *outfile;
    err = _wfopen_s(&outfile, filename, L"wb");
    if (err)
        return false;

    bool ret = SaveToNewPicture(outfile, pinfo, type);

    fclose(outfile);
    return ret;
}

bool SaveToNewPicture(const char *filename, ImageInfo *pinfo, E_ImageType type)
{
    errno_t err;
    FILE *outfile;
    err = fopen_s(&outfile, filename, "wb");
    if (err)
        return false;

    bool ret = SaveToNewPicture(outfile, pinfo, type);

    fclose(outfile);
    return ret;
}


////////////////////////////////////////////////////////////////////////////////


bool StretchPixels(const ImageInfo *in, ImageInfo *out)
{
    if (!in || !out)
        return false;
    if (in->component <= 0 || in->width <= 0 || in->height <= 0 || !in->ppixels ||
        out->height <= 0 || out->width <= 0)
        return false;

    float x_scale = (float)out->width / in->width;
    float y_scale = (float)out->height / in->height;

    if (x_scale < 1.0f && y_scale < 1.0f)
        return StretchPixels_Shrink(in, out);
    else
        return StretchPixels_Expand(in, out);
}

bool StretchPixels_Shrink(const ImageInfo * in, ImageInfo * out)
{
    // 局部均值缩小

    const float k1 = (float)in->width / out->width;      //  > 1
    const float k2 = (float)in->height / out->height;    //  > 1
    const int block = (int)k1 * (int)k2;
    const int new_size = out->width * out->height;

    out->component = in->component;
    unsigned char *outBuf = new unsigned char[new_size * out->component];

    if (in->component == 3)
    {
        for (int y = 0; y < out->height; ++y)
        {
            for (int x = 0; x < out->width; ++x)
            {
                int r = 0, g = 0, b = 0;
                for (int j = 0; j < (int)k2; ++j)
                    for (int i = 0; i < (int)k1; ++i)
                    {
                        int p = ((int)(y*k2 + j)*in->width + (int)(x*k1 + i)) * 3;
                        assert(p < in->width * in->height * 3);
                        r += in->ppixels[p];
                        g += in->ppixels[p + 1];
                        b += in->ppixels[p + 2];
                    }
                int tar = (y * out->width + x) * 3;
                assert(r / block < 0x100);
                assert(g / block < 0x100);
                assert(b / block < 0x100);
                outBuf[tar] = r / block;
                outBuf[tar + 1] = g / block;
                outBuf[tar + 2] = b / block;
            }
        }
    }
    else if (in->component == 4)
    {
        for (int y = 0; y < out->height; ++y)
        {
            for (int x = 0; x < out->width; ++x)
            {
                int r = 0, g = 0, b = 0, a = 0;
                for (int j = 0; j < (int)k2; ++j)
                    for (int i = 0; i < (int)k1; ++i)
                    {
                        int p = ((int)(y*k2 + j)*in->width + (int)(x*k1 + i)) * 4;

                        assert(p < in->width * in->height * 4);
                        r += in->ppixels[p];
                        g += in->ppixels[p + 1];
                        b += in->ppixels[p + 2];
                        a += in->ppixels[p + 3];
                    }
                int tar = (y * out->width + x) * 4;
                assert(r / block < 0x100);
                assert(g / block < 0x100);
                assert(b / block < 0x100);
                assert(a / block < 0x100);
                outBuf[tar] = r / block;
                outBuf[tar + 1] = g / block;
                outBuf[tar + 2] = b / block;
                outBuf[tar + 3] = a / block;
            }
        }
    }
    else
        return false;
    
    out->ppixels = outBuf;
    return true;
}

bool StretchPixels_Expand(const ImageInfo * in, ImageInfo * out)
{
    // 双线性插值

    const float k1 = (float)out->width / in->width;      //  > 1
    const float k2 = (float)out->height / in->height;    //  > 1
    const int new_size = out->width * out->height;

    out->component = in->component;
    unsigned char *outBuf = new unsigned char[new_size * out->component];

    for (int y = 0; y < in->height; ++y)
        for (int x = 0; x < in->width; ++x)
        {
            int top = y * k2;
            int bottom = (y + 1) * k2;
            int left = x * k1;
            int right = (x + 1) * k1;

            int block = (bottom - top) * (right - left);

            unsigned char *p1 = &in->ppixels[(y * in->width + x) * in->component];
            unsigned char *p2 = x < in->width - 1 ? p1 + in->component : p1;
            unsigned char *p3 = &in->ppixels[((y < in->height - 1 ? y + 1 : in->height - 1) * in->width + x) * in->component];
            unsigned char *p4 = x < in->width - 1 ? p3 + in->component : p3;

            for (int j = top; j < bottom; ++j)
                for (int i = left; i < right; ++i)
                {
                    int tar_p = (j * out->width + i) * out->component;
                    float f1 = (float)(bottom - j) * (right - i) / block;       // (1-u)(1-v)
                    float f2 = (float)(bottom - j) * (i - left) / block;        // u(1-v)
                    float f3 = (float)(j - top) * (right - i) / block;          // (1-u)v
                    float f4 = (float)(j - top) * (i - left) / block;           // uv

                    int r = f1 * *p1 + f2 * *p2 + f3 * *p3 + f4 * *p4;
                    int g = f1 * *(p1+1) + f2 * *(p2+1) + f3 * *(p3+1) + f4 * *(p4+1);
                    int b = f1 * *(p1+2) + f2 * *(p2+2) + f3 * *(p3+2) + f4 * *(p4+2);
                    assert(r < 0x100);
                    assert(g < 0x100);
                    assert(b < 0x100);
                    outBuf[tar_p] = r;
                    outBuf[tar_p+1] = g;
                    outBuf[tar_p+2] = b;
                }

        }

    out->ppixels = outBuf;
    return true;
}

int OtsuThresholding(const int *histogram, int total)
{
    // 大津二值化法
    // w1w2(μ1-μ2)^2
    // w1 - 小于阈值的像素比例
    // w2 - 大于阈值的像素比例
    // μ1 - 小于阈值像素的平均值
    // μ2 - 大于阈值像素的平均值

    // histogram - 像素的灰阶图
    // total     - 总像素数

    int sum = 0;
    for (int i = 1; i < 256; ++i)
        sum += i * histogram[i];
    int sumB = 0;
    int wB = 0;
    int wF = 0;
    float mB;
    float mF;
    float max = 0.0f;
    float between = 0.0f;
    float threshold1 = 0.0f;
    float threshold2 = 0.0f;
    for (int i = 0; i < 256; ++i) {
        wB += histogram[i];
        if (wB == 0)
            continue;
        wF = total - wB;
        if (wF == 0)
            break;
        sumB += i * histogram[i];
        mB = sumB / wB;
        mF = (sum - sumB) / wF;
        between = wB * wF * (mB - mF) * (mB - mF);
        if (between >= max)
        {
            threshold1 = i;
            if (between > max)
                threshold2 = i;
            max = between;
        }
    }
    return (int)((threshold1 + threshold2) / 2.0f);
}

bool CreateGray(const ImageInfo * in, ImageInfo * out)
{
    // 将RGB转为灰度
    // Gray = R*0.299 + G*0.587 + B*0.114 => (R*38 + G*75 + B*15) >> 7

    out->width = in->width;
    out->height = in->height;
    out->component = 1;

    if (in->component == 4 || in->component == 3)
    {
        out->ppixels = new unsigned char[out->width * out->height];
        for (int i = 0; i < in->width * in->height; ++i)
        {
            int offset = i * in->component;
            int gray = in->ppixels[offset] * 38 +
                       in->ppixels[offset + 1] * 75 +
                       in->ppixels[offset + 2] * 15;
            gray >>= 7;
            out->ppixels[i] = gray;
        }
        return true;
    }
    else
        return false;
}
#endif


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
        bool CopyPixelInLine(int dstLineOffset, int dstRowOffset, ImageImpl * srcObj, int srcLineOffset, int srcRowOffset, int cnt = -1);
        void ModifyPixels(std::function<void(int row, int col, Pixel &)> func);
        void WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const;
        bool StretchTo(int width, int height);
        int  OtsuThresholding() const;
        Image CreateGray() const;
        bool RemoveAlpha();
        bool AddAlpha();
        
        // 添加/移除 lapha 通道

        // ico / gif / mpng / tiff / webp / 调色板

    private:
        E_ImageType GetImageType(const string_t & filename);

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

void Alisa::Image::Clear()
{
    return Impl->Clear();
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

bool Alisa::Image::StretchTo(int width, int height)
{
    return Impl->StretchTo(width, height);
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

bool Alisa::ImageImpl::StretchTo(int width, int height)
{
    assert(0); 
    return false;
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
        return E_ImageType_Unknown;

    unsigned char buf[4];
    fread_s(buf, sizeof(buf), sizeof(buf), 1, infile);

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

    if (bfh.bfType != 'MB')     // 小端序
        return false;

    if (bih.biBitCount < 8)
        return false;

    assert(bih.biBitCount == 24 || bih.biBitCount == 32);   // 调色板再说

    img->BaseInfo.Height = abs(bih.biHeight);
    img->BaseInfo.Width = bih.biWidth;
    img->BaseInfo.Component = bih.biBitCount >> 3;
    int row_stride = bih.biWidth * (bih.biBitCount >> 3);
    int row_expand = (row_stride + 3) & ~3;             // bmp的行4字节对齐

    fseek(infile, 0, SEEK_END);
    int bytesize = ftell(infile) - bfh.bfOffBits;       // bfh.bfSize 字段靠不住
    assert(bytesize >= img->BaseInfo.Height * row_stride);
    unsigned char *ppixels = new unsigned char[bytesize];

    fseek(infile, bfh.bfOffBits, SEEK_SET);
    for (int i = 0; i < img->BaseInfo.Height; ++i)
    {
        long dummy;
        // 读取时忽略掉每行填充的几个字节
        if (bih.biHeight > 0)
            fread_s(ppixels + i * row_stride, row_stride, 1, row_stride, infile);
        else
            fread_s(ppixels + (img->BaseInfo.Height - 1 - i) * row_stride, row_stride, 1, row_stride, infile);
        fread_s(&dummy, sizeof(long), 1, row_expand - row_stride, infile);

    }
    fclose(infile);
    infile = NULL;

    // 内存中像素点的颜色分量顺序是RGB(A)，bmp文件中的顺序是BGR(A)
    unsigned char *ptr = ppixels;
    for (int i = 0; i < img->BaseInfo.Height * img->BaseInfo.Width; ++i)
    {
        // 将B和R的分量对换
        unsigned char tmp;
        tmp = ptr[0]; ptr[0] = ptr[2]; ptr[2] = tmp;
        ptr += img->BaseInfo.Component;
    }


    img->Pixels.resize(img->BaseInfo.Height);
    for (size_t i = 0; i < img->Pixels.size(); ++i)
        img->Pixels[i].resize(img->BaseInfo.Width);

    for (int i = 0; i < img->BaseInfo.Height; ++i)
    {
        for (int k = 0; k < img->BaseInfo.Width; ++k)
        {
            Pixel & p = img->Pixels[i][k];
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
    int aligned_width = (row_stride + 3) & ~3;          // bmp 每行需要4字节对齐

    _BITMAPFILEHEADER bfh = { 0 };
    bfh.bfType = 'MB';  // 小端
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
    errno_t err = fopen_s(&infile, filename.c_str(), "wb");
#endif
    if (err)
        return false;

    fwrite(&bfh, sizeof(_BITMAPFILEHEADER), 1, outfile);
    fwrite(&bih, sizeof(_BITMAPINFOHEADER), 1, outfile);

    // 内存中像素点的颜色分量顺序是RGB(A)，bmp文件中的顺序是BGR(A)
    unsigned char *buffer = new unsigned char[row_stride];
    for (int i = 0; i < img->BaseInfo.Height; ++i)
    {
        const auto & row = img->Pixels[i];

        if (img->BaseInfo.Component == PixelType_RGB)
            for (int k = 0; k < img->BaseInfo.Width; ++k)
            {
                buffer[k * 3]     = row[k].B;
                buffer[k * 3 + 1] = row[k].G;
                buffer[k * 3 + 2] = row[k].R;
            }

        else if (img->BaseInfo.Component == PixelType_RGBA)
            for (int k = 0; k < img->BaseInfo.Width; ++k)
            {
                buffer[k * 4]     = row[k].B;
                buffer[k * 4 + 1] = row[k].G;
                buffer[k * 4 + 2] = row[k].R;
                buffer[k * 4 + 3] = row[k].A;
            }

        else
        {
            assert(0);
            delete[] buffer;
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

    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

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
        fprintf(stderr, "错误码：%d\n", iRetVal);
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
    // 绑定libpng和文件流
    //
    png_init_io(png_ptr, infile);
    png_read_info(png_ptr, info_ptr);

    //
    // 获取文件头信息
    //
    int bit_depth, color_type;
    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr,
        &width, &height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    //
    // 按颜色格式读取为RGBA
    //

    int pixel_byte = color_type == PNG_COLOR_TYPE_RGB ? 3 : 4;

    //要求转换索引颜色到RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    //要求位深度强制8bit
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth<8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    //要求位深度强制8bit
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    //灰度必须转换成RGB
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    //
    // 分配像素缓冲区
    //
    pPixels = new unsigned char[width * height * pixel_byte];
    lines = new unsigned char*[height * sizeof(unsigned char*)]; //列指针

    png_int_32 h = height - 1;
    png_int_32 i = 0;
    while (h >= 0)//逆行序读取，因为位图是底到上型
    {
        lines[i] = &pPixels[h * width * pixel_byte];
        --h;
        ++i;
    }

    //
    // 读取像素
    //
    png_read_image(png_ptr, (png_bytepp)lines);

    img->Clear();
    img->BaseInfo.Height = height;
    assert(img->BaseInfo.Height > 0);
    img->BaseInfo.Width = width;
    img->BaseInfo.Component = pixel_byte;
    
    img->Pixels.resize(height);
    for (size_t i = 0; i < img->Pixels.size(); ++i)
    {
        img->Pixels[i].resize(width);
        for (size_t k = 0; k < img->Pixels[i].size(); ++k)
        {
            Pixel & p = img->Pixels[i][k];
            p.R = *(lines[img->Pixels.size() - 1 - i] + k * pixel_byte);
            p.G = *(lines[img->Pixels.size() - 1 - i] + k * pixel_byte + 1);
            p.B = *(lines[img->Pixels.size() - 1 - i] + k * pixel_byte + 2);
            p.A = pixel_byte == PixelType_RGBA ? *(lines[img->Pixels.size() - 1 - i] + k * pixel_byte + 3) : 0xff;
        }
    }


    //
    // 释放资源
    //
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    delete[] lines;
    delete[] pPixels;
    return true;
}

bool Alisa::ImageCodec::EncodePng(const string_t & filename, const ImageImpl * img)
{
    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

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
        fprintf(stderr, "错误码：%d\n", iRetVal);
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
    //设置PNG文件头
    //
    png_set_IHDR(
        png_ptr,
        info_ptr,
        img->BaseInfo.Width,
        img->BaseInfo.Height,
        8,                              //颜色深度,
        img->BaseInfo.Component == PixelType_RGBA ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,//颜色类型
        PNG_INTERLACE_NONE,             //不交错。交错: PNG_INTERLACE_ADAM7
        PNG_COMPRESSION_TYPE_DEFAULT,   //压缩方式
        PNG_FILTER_TYPE_DEFAULT         //什么过滤? 默认填 PNG_FILTER_TYPE_DEFAULT
    );
    //设置打包信息
    png_set_packing(png_ptr);
    //写入文件头
    png_write_info(png_ptr, info_ptr);

    lines = new unsigned char*[img->BaseInfo.Height * sizeof(unsigned char*)]; //列指针

    png_int_32 i = 0;
    png_int_32 h = img->BaseInfo.Height - 1;
    while (h >= 0)//逆行序读取，因为位图是底到上型
    {
        lines[i] = new unsigned char[img->BaseInfo.Width * img->BaseInfo.Component];
        for (int w = 0; w < img->BaseInfo.Width; ++w)
        {
            lines[i][w * img->BaseInfo.Component]     = img->Pixels[h][w].R;
            lines[i][w * img->BaseInfo.Component + 1] = img->Pixels[h][w].G;
            lines[i][w * img->BaseInfo.Component + 2] = img->Pixels[h][w].B;
            if (img->BaseInfo.Component == PixelType_RGBA)
                lines[i][w * img->BaseInfo.Component + 3] = img->Pixels[h][w].A;
        }
        ++i, --h;
    }

    //
    // 写入像素
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
        fprintf(stderr, "jpg打开失败\r\n");
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

        // 按bmp/png把行像素倒置
        row_arr = new JSAMPROW[cinfo.image_height];
        for (JDIMENSION i = 0; i < cinfo.image_height; ++i)
            row_arr[i] = (JSAMPROW)(ppixels + (cinfo.image_height - i - 1) * row_stride);
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
        fprintf(stderr, "jpg存储失败\r\n");
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
    errno_t err = fopen_s(&infile, filename.c_str(), "wb");
#endif
    if (err)
        return false;


    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = img->BaseInfo.Width;
    cinfo.image_height = img->BaseInfo.Height;
    cinfo.in_color_space = JCS_RGB;
    cinfo.input_components = PixelType_RGB;     // RGB

    // alpha blend
    // 像素顺序为RGBA
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

    // 储存的时候倒着来
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &translate[(cinfo.image_height - cinfo.next_scanline - 1) * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    if (translate)
        delete[] translate;

    return true;
}
