
#include <assert.h>
#include <vector>
#include <stdio.h>
#include <string>
#include "picture.h"
#include "featurevector.h"
#include "OpenFiles.h"
#include <Windows.h>
#include "window.h"

int main3(int argc, wchar_t **argv)
{
    std::vector<std::wstring> path, out;
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
            path.push_back(std::wstring(argv[i]));
        GetSubFileList(path, out);

        FeatureVector fv;
        fv.Initialize();

        for (int i = 0; i < out.size(); ++i)
        {
            ImageInfo info;
            bool res;
            res = GetImageRawData(out[i].c_str(), &info);
            if (!res)
                continue;

            fv.AddPicture(out[i].c_str(), &info);
        }

        fv.DivideGroup(nullptr);
    }
    else
        fprintf(stderr, "Usage:\r\n");

    system("pause");
    return 0;
}

int main4(int argc, wchar_t **argv)
{
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

            //s_img.width = 150;
            //s_img.height = 150 *img.height / img.width;            
            //StretchPixels(&img, &s_img);
            //std::wstring n(out[i]);
            //n.append(L"_sNew.bmp");
            //SaveToNewPicture(n.c_str(), &s_img, E_ImageType_Bmp);

            //l_img.width = 1534;
            //l_img.height = 1534 * img.height / img.width;
            l_img.width = 500;
            l_img.height = 500;
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