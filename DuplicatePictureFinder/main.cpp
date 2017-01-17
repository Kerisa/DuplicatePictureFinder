
#include <assert.h>
#include <stdio.h>
#include <string>
#include "picture.h"
#include "featurevector.h"


int main(int argc, char **argv)
{
    char filename[16];
    FILE *filegroup[16] = { 0 };
    errno_t err = 0;
    
    FeatureVector fv;
    fv.Initialize();

    for (int i = 0; i < _countof(filegroup); ++i)
    {
        sprintf_s(filename, sizeof(filename), "%02d.jpg", i + 1);
        if (i + 1 == 9)
            strcpy_s(filename, sizeof(filename), "09.png");

        ImageInfo info;
        bool res;
        res = GetImageRawData(filename, &info);
        if (!res)
            continue;

        fv.AddPicture(filename, &info);
    }

    fv.DivideGroup(nullptr);
    
    
    for (int i = 0; i < _countof(filegroup); ++i)
        if (filegroup[i])
            fclose(filegroup[i]);

    system("pause");
    return 0;
}