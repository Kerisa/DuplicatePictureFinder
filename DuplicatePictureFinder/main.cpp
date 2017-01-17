
#include <string>
#include "picture.h"

int main(int argc, char **argv)
{
    ImageInfo info;
    bool res;
    res = GetImageRawData(argv[1], &info);
    assert(res);

    std::string name;
    name.assign(argv[1]);
    name += ".New.bmp";
    res = SaveToNewPicture(name.c_str(), &info, E_ImageType_Bmp);
    assert(res);

    name.assign(argv[1]);
    name += ".New.jpg";
    res = SaveToNewPicture(name.c_str(), &info, E_ImageType_Jpg);
    assert(res);

    name.assign(argv[1]);
    name += ".New.png";
    res = SaveToNewPicture(name.c_str(), &info, E_ImageType_Png);
    assert(res);
    
    return 0;
}