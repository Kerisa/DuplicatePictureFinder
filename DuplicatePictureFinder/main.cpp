
#include "picture.h"

int main(int argc, char **argv)
{
    //FILE *fpng, *fbmp, *fjpg;
    errno_t err;
    //err = fopen_s(&fpng, "1.png", "rb");
    //assert(!err);
    //err = fopen_s(&fbmp, "1.bmp", "rb");
    //assert(!err);
    //err = fopen_s(&fjpg, "1.jpg", "rb");
    //assert(!err);

    FILE *Img;
    err = fopen_s(&Img, argv[1], "rb");
    assert(!err);

    int type =  GetImageType(Img);
    switch (type)
    {
    case E_ImageType_Bmp:
    {
        ImageInfoBmp ib;
        bool ret = GetImageRawData(Img, &ib);
        assert(ret);
        FILE *out_bmp;

        ret = fopen_s(&out_bmp, "bmptojpg.jpg", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_bmp, (ImageInfoJpg*)&ib);
        assert(ret);
        fclose(out_bmp);

        ret = fopen_s(&out_bmp, "bmptopng.png", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_bmp, (ImageInfoPng*)&ib);
        assert(ret);
        fclose(out_bmp);

        ret = fopen_s(&out_bmp, "bmptobmp.bmp", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_bmp, &ib);
        assert(ret);
        fclose(out_bmp);
        break;
    }

    case E_ImageType_Jpg:
    {
        ImageInfoJpg ij;
        bool ret = GetImageRawData(Img, &ij);
        assert(ret);
        FILE *out_jpg;

        ret = fopen_s(&out_jpg, "jpgtojpg.jpg", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_jpg, &ij);
        assert(ret);
        fclose(out_jpg);

        ret = fopen_s(&out_jpg, "jpgtopng.png", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_jpg, (ImageInfoPng*)&ij);
        assert(ret);
        fclose(out_jpg);

        ret = fopen_s(&out_jpg, "jpgtobmp.bmp", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_jpg, (ImageInfoBmp*)&ij);
        assert(ret);
        fclose(out_jpg);
        break;
    }

    case E_ImageType_Png:
    {
        ImageInfoPng ip;
        bool ret = GetImageRawData(Img, &ip);
        assert(ret);
        FILE *out_png;

        ret = fopen_s(&out_png, "pngtojpg.jpg", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_png, (ImageInfoJpg*)&ip);
        assert(ret);
        fclose(out_png);
        
        ret = fopen_s(&out_png, "pngtopng.png", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_png, &ip);
        assert(ret);
        fclose(out_png);
                
        ret = fopen_s(&out_png, "pngtobmp.bmp", "wb");
        assert(!ret);
        ret = SaveToNewPicture(out_png, (ImageInfoBmp*)&ip);
        assert(ret);
        fclose(out_png);
        break;
    }

    default:
        assert(0);
        break;
    }

    //fclose(fpng);
    //fclose(fbmp);
    //fclose(fjpg);
    fclose(Img);

    return 0;
}