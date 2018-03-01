
#include <assert.h>
#include <vector>
#include <stdio.h>
#include <string>
#include <time.h>
#include "picture.h"
#include "featurevector.h"
#include "OpenFiles.h"
#include <Windows.h>
#include "window.h"

int main8(int argc, wchar_t **argv)
{
    // 测试新的读取、保存图像函数
    std::vector<std::wstring> path, out;
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
            path.push_back(std::wstring(argv[i]));
        GetSubFileList(path, out);
        
        for (int i = 0; i < out.size(); ++i)
        {
            printf("process %d...\n", i);
            if (out[i].find(L"__dup__") != std::wstring::npos)
            {
                printf("    skip dup image\n");
                continue;
            }
            printf("    read image...\n");
            Alisa::Image image;
            image.Open(out[i]);
            
            auto copy = out[i];
            copy = copy.substr(0, copy.find_last_of('.'));

            //{
            //    image.RemoveAlpha();
            //    image.AddAlpha();
            //}

            printf("    save bmp...\n");
            image.SaveTo(copy + L"__dup__.bmp", Alisa::E_ImageType_Bmp);
            printf("    save jpg...\n");
            image.SaveTo(copy + L"__dup__.jpg", Alisa::E_ImageType_Jpg);
            printf("    save png...\n");
            image.SaveTo(copy + L"__dup__.png", Alisa::E_ImageType_Png);
            printf("    complete.\n");
        }
    }
    else
        fprintf(stderr, "Usage:\r\n");

    system("pause");
    return 0;
}

int main7(int argc, wchar_t **argv)
{
    // 使用 isodata
    std::vector<std::wstring> path, out;
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
            path.push_back(std::wstring(argv[i]));
        GetSubFileList(path, out);

        Alisa::ImageFeatureVector fv;
        fv.Initialize();

        for (int i = 0; i < out.size(); ++i)
        {
            //ImageInfo info;
            //bool res;
            //res = GetImageRawData(out[i].c_str(), &info);
            //if (!res)
            //    continue;

            Alisa::Image img;
            if (!img.Open(out[i]))
            {
                assert(0);
                continue;
            }

            fv.AddPicture(out[i].c_str(), img);
        }

        // 使用 isodata
        fv.IsoData();
    }
    else
        fprintf(stderr, "Usage:\r\n");

    system("pause");
    return 0;
}

int main3(int argc, wchar_t **argv)
{
    // 计算巴氏距离并分组
    std::vector<std::wstring> path, out;
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
            path.push_back(std::wstring(argv[i]));
        GetSubFileList(path, out);

        Alisa::ImageFeatureVector fv;
        fv.Initialize();

        for (int i = 0; i < out.size(); ++i)
        {
            //ImageInfo info;
            //bool res;
            //res = GetImageRawData(out[i].c_str(), &info);
            //if (!res)
            //    continue;
            Alisa::Image img;
            if (!img.Open(out[i]))
            {
                assert(0);
                continue;
            }

            fv.AddPicture(out[i].c_str(), img);
        }

        // svd
        //fv.svd();

        fv.DivideGroup();
    }
    else
        fprintf(stderr, "Usage:\r\n");

    system("pause");
    return 0;
}

int main_test_pixel(int argc, wchar_t **argv)
{
    bool b = false;
    Alisa::Image img;
    b = img.Open(argv[1]);
    assert(b);
    img.ModifyPixels([](int r, int c, Alisa::Pixel & p) {
        if (r == c && c == 0)
        {
            p.A = 0xff; p.R = 0xff; p.G = 0; p.B = 0;
        }
    });
    b = img.SaveTo(string_t(argv[1]) + TEXT("1_1.png"), Alisa::E_ImageType_Png);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("1_2.jpg"), Alisa::E_ImageType_Jpg);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("1_3.bmp"), Alisa::E_ImageType_Bmp);
    assert(b);
    img.RemoveAlpha();
    b = img.SaveTo(string_t(argv[1]) + TEXT("1_4.png"), Alisa::E_ImageType_Png);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("1_5.bmp"), Alisa::E_ImageType_Bmp);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("1_6.jpg"), Alisa::E_ImageType_Jpg);
    assert(b);
    //////
    b = img.Open(argv[2]);
    assert(b);
    img.ModifyPixels([](int r, int c, Alisa::Pixel & p) {
        if (r == c && c == 0)
        {
            p.A = 0xff; p.R = 0xff; p.G = 0; p.B = 0;
        }
    });
    b = img.SaveTo(string_t(argv[1]) + TEXT("2_1.png"), Alisa::E_ImageType_Png);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("2_2.jpg"), Alisa::E_ImageType_Jpg);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("2_3.bmp"), Alisa::E_ImageType_Bmp);
    assert(b);
    img.RemoveAlpha();
    b = img.SaveTo(string_t(argv[1]) + TEXT("2_4.png"), Alisa::E_ImageType_Png);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("2_5.bmp"), Alisa::E_ImageType_Bmp);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("2_6.jpg"), Alisa::E_ImageType_Jpg);
    assert(b);
    //////
    b = img.Open(argv[3]);
    assert(b);
    img.ModifyPixels([](int r, int c, Alisa::Pixel & p) {
        if (r == c && c == 0)
        {
            p.A = 0xff; p.R = 0xff; p.G = 0; p.B = 0;
        }
    });
    b = img.SaveTo(string_t(argv[1]) + TEXT("3_1.png"), Alisa::E_ImageType_Png);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("3_2.jpg"), Alisa::E_ImageType_Jpg);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("3_3.bmp"), Alisa::E_ImageType_Bmp);
    assert(b);
    img.RemoveAlpha();
    b = img.SaveTo(string_t(argv[1]) + TEXT("3_4.png"), Alisa::E_ImageType_Png);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("3_5.bmp"), Alisa::E_ImageType_Bmp);
    assert(b);
    b = img.SaveTo(string_t(argv[1]) + TEXT("3_6.jpg"), Alisa::E_ImageType_Jpg);
    assert(b);
    return 0;
}

#if 0
int main4(int argc, wchar_t **argv)
{
    // 测试缩放图像
    std::vector<std::wstring> path, out;
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
            path.push_back(std::wstring(argv[i]));
        GetSubFileList(path, out);

        for (int i = 0; i < out.size(); ++i)
        {
            ImageInfo img, s_img, l_img;
            bool res = GetImageRawData(out[i].c_str(), &img);
            if (!res)
                continue;

            s_img.width = 150;
            s_img.height = 150 *img.height / img.width;            
            StretchPixels(&img, &s_img);
            std::wstring n(out[i]);
            n.append(L"_sNew.bmp");
            SaveToNewPicture(n.c_str(), &s_img, E_ImageType_Bmp);

            l_img.width = 1534;
            l_img.height = 1534 * img.height / img.width;
            //l_img.width = 500;
            //l_img.height = 500;
            StretchPixels(&img, &l_img);
            std::wstring nn(out[i]);
            nn.append(L"_lNew.bmp");
            SaveToNewPicture(nn.c_str(), &l_img, E_ImageType_Bmp);
        }
    }
    return 0;
}

int main5(int argc, wchar_t **argv)
{
    // 输出二值化结果为图片
    std::vector<std::wstring> path, out;
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
            path.push_back(std::wstring(argv[i]));
        GetSubFileList(path, out);

        for (int i = 0; i < out.size(); ++i)
        {
            ImageInfo img, s_img, g_img;
            bool res = GetImageRawData(out[i].c_str(), &img);
            if (!res)
                continue;

            s_img.width = s_img.height = 150;
            res = StretchPixels(&img, &s_img);
            assert(res);
            res = CreateGray(&s_img, &g_img);
            assert(res);

            assert(g_img.component == 1);
            int *hist = new int[256];
            for (int i = 0; i < g_img.width * g_img.height; ++i)
                ++hist[g_img.ppixels[i]];

            int threshold = OtsuThresholding(hist, g_img.width * g_img.height);

            delete[] hist;

            // 将小图二值化
            for (int i = 0; i < s_img.width * s_img.height; ++i)
            {
                unsigned char val = g_img.ppixels[i] < threshold ? 0 : 255;
                s_img.ppixels[i * s_img.component] = val;
                s_img.ppixels[i * s_img.component + 1] = val;
                s_img.ppixels[i * s_img.component + 2] = val;
                if (s_img.component == 4)
                    s_img.ppixels[i * s_img.component + 3] = 255;
            }

            std::wstring nn(out[i]);
            nn.append(L"_Two.bmp");
            SaveToNewPicture(nn.c_str(), &s_img, E_ImageType_Bmp);
        }
    }
    return 0;
}

int main6(int argc, wchar_t **argv)
{
    // 比较图像特征值并分组
    std::vector<std::wstring> path, out;
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
            path.push_back(std::wstring(argv[i]));
        GetSubFileList(path, out);

        std::vector<std::vector<std::pair<std::wstring, ImageInfo>>> grayGroup;

        for (int i = 0; i < out.size(); ++i)
        {
            ImageInfo img, s_img, g_img;
            bool res = GetImageRawData(out[i].c_str(), &img);
            if (!res)
                continue;

            s_img.width = s_img.height = 100;
            res = StretchPixels(&img, &s_img);
            assert(res);
            res = CreateGray(&s_img, &g_img);
            assert(res);

            assert(g_img.component == 1);
            int *hist = new int[256];
            for (int i = 0; i < g_img.width * g_img.height; ++i)
                ++hist[g_img.ppixels[i]];

            // 二值化
            int threshold = OtsuThresholding(hist, g_img.width * g_img.height);
            for (int i = 0; i < s_img.width * s_img.height; ++i)
                g_img.ppixels[i] = g_img.ppixels[i] < threshold ? 0 : 255;

            delete[] hist;

            std::vector<std::pair<std::wstring, ImageInfo>> tmp;
            tmp.push_back(std::make_pair(out[i], std::move(g_img)));
            grayGroup.push_back(std::move(tmp));
        }

        long first = clock();
        for (int i = 0; i < grayGroup.size() - 1; ++i)
        {
            if (grayGroup[i].empty())
                continue;
            long start = clock();
            for (int j = i + 1; j < grayGroup.size(); ++j)
            {
                if (grayGroup[j].empty())
                    continue;
                int xor_cnt = 0;
                for (int k = 0; k < 10000; ++k) // 尺寸为100*100
                    if (grayGroup[i][0].second.ppixels[k] ^ grayGroup[j][0].second.ppixels[k])
                        ++xor_cnt;      // 越小越相似

                if (xor_cnt < 3000)
                {
                    grayGroup[i].push_back(std::move(grayGroup[j][0]));
                    grayGroup[j].clear();
                }

                //fprintf(stderr, "%d <=> %d\r\n", i, j);
            }
            fprintf(stderr, "%d time:%d\r\n", i, clock()-start);
        }

        setlocale(LC_ALL, "");
        fprintf(stderr, "total time:%d\r\n", clock() - first);
        for (int i = 0, cnt = 0; i < grayGroup.size(); ++i)
            if (!grayGroup[i].empty())
            {
                printf("Group %d:\n", ++cnt);
                for (int j = 0; j < grayGroup[i].size(); ++j)
                    printf("%ws\n", grayGroup[i][j].first.c_str());
            }
        setlocale(LC_ALL, "C");
    }
    return 0;
}
#endif