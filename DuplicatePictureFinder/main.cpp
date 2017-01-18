
#include <assert.h>
#include <vector>
#include <stdio.h>
#include <string>
#include "picture.h"
#include "featurevector.h"
#include "OpenFiles.h"


int main(int argc, char **argv)
{
    std::vector<std::string> path, out;
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
            path.push_back(std::string(argv[i]));
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