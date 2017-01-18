
#include <assert.h>
#include "featurevector.h"
#include "picture.h"
#include "crc.h"

bool FeatureVector::CrcInitialized = false;

FeatureVector::FeatureVector()
{
    if (!CrcInitialized)
    {
        create_crc_table();
        CrcInitialized = true;
    }
}

bool FeatureVector::Initialize(int _iterations, int)
{
    Clear();
    mIterations = _iterations;
    return true;
}

void FeatureVector::Clear()
{
    mIterations = 0;
    mData.clear();
    std::map<unsigned int, FeatureData>().swap(mData);
    mGroup.clear();
    std::vector<std::vector<SingleDataMap::iterator>>().swap(mGroup);
}

bool FeatureVector::AddPicture(const char * filename, const ImageInfo * pinfo)
{
    assert(mcDivideRegion > 0);

    if (!filename || !pinfo || 
        pinfo->height <= 0 || pinfo->width <= 0 || pinfo->component <= 0 ||
        pinfo->ppixels == nullptr)
        return false;

    unsigned int crc = CRC32_4((const unsigned char*)filename, 0, strlen(filename));
    assert(mData.find(crc) == mData.end());
    auto res = mData.insert(std::make_pair(crc, FeatureData()));
    FeatureData & data = res.first->second;
    data.filename.assign(filename);
    data.pixelcount = pinfo->width * pinfo->height;
    
    int dimension = 0x100 / mcDivideRegion;
    data.histogram = new int[dimension * dimension * dimension];
    memset(data.histogram, 0, dimension * dimension * dimension * sizeof(int));

    if (pinfo->component == 3 || pinfo->component == 4)
        for (int i = 0; i < pinfo->width * pinfo->height * pinfo->component; i += pinfo->component)
        {
            unsigned char R = pinfo->ppixels[i];
            unsigned char G = pinfo->ppixels[i + 1];
            unsigned char B = pinfo->ppixels[i + 2];

            ++data.histogram[R / mcDivideRegion * dimension * dimension +
                             G / mcDivideRegion * dimension +
                             B / mcDivideRegion];
        }

    else
    {
        mData.erase(crc);
        assert(0);
        return false;
    }

    return true;
}

bool FeatureVector::DivideGroup(fn_image_cmp_result callback)
{
    // 最初每张图像各成一组
    for (auto it = mData.begin(); it != mData.end(); ++it)
    {
        std::vector<std::map<unsigned int, FeatureData>::iterator> tmp;
        tmp.push_back(it);
        mGroup.push_back(tmp);
    }

    while (mIterations--)
    {
        for (int i = 0; i < mGroup.size(); ++i)
        {
            for (int j = i + 1; j < mGroup.size(); ++j)
            {
                float distance = CalcGroup(mGroup[i], mGroup[j]);
#ifdef _DEBUG
                if (!mGroup[i].empty() && !mGroup[j].empty())
                    fprintf(stderr, "Group[%d](%d) to Group[%d](%d): Distance=%0.6f\r\n",
                        i, mGroup[i].size(), j, mGroup[j].size(), distance);
#endif
                if (distance > mcThreshold)
                {
                    // 相似
                    mGroup[i].insert(mGroup[i].end(), mGroup[j].begin(), mGroup[j].end());
                    mGroup[j].clear();
                }
            }
        }
    }

#ifdef _DEBUG
    FILE *debug_out;
    fopen_s(&debug_out, "debug_out.log", "wb");
    int gcnt = 0;
    for (int i = 0; i < mGroup.size(); ++i)
    {
        if (mGroup[i].empty())
            continue;

        fprintf(debug_out, "Group: %d, element: %d\r\n", gcnt, mGroup[i].size());
        for (auto it = mGroup[i].begin(); it != mGroup[i].end(); ++it)
            fprintf(debug_out, "    %s\r\n", (*it)->second.filename.c_str());
        fprintf(debug_out, "\r\n");

        ++gcnt;
    }
    fclose(debug_out);
#endif

    return true;
}

float FeatureVector::Calc(const FeatureData &src, const FeatureData &dst)
{
    float sum = 0.0f;
    int dimension = 0x100 / mcDivideRegion;
    int times = dimension * dimension * dimension;  // 向量的维数

    for (int i = 0; i < times; ++i)
        sum += sqrt(((float)src.histogram[i] / times) * ((float)dst.histogram[i] / times));
    
#ifdef _DEBUG
    fprintf(stderr, "%s(%d) - %s(%d) : %.5f",
        src.filename.c_str(), src.group,
        dst.filename.c_str(), dst.group, sum);
#endif

    return sum;
}

float FeatureVector::CalcGroup(
    std::vector<SingleDataMap::iterator> &src,
    std::vector<SingleDataMap::iterator> &dst
)
{
    float sum = 0.0f;
    int dimension = 0x100 / mcDivideRegion;
    int times = dimension * dimension * dimension;  // 向量的维数

    if (src.empty() || dst.empty())
        return sum;

    for (int i = 0; i < times; ++i)
    {
        float s_tot = 0.0f, d_tot = 0.0f;
        for (int j = 0; j < src.size(); ++j)
            s_tot += (float)src[j]->second.histogram[i] / src.size() / src[j]->second.pixelcount;
        for (int j = 0; j < dst.size(); ++j)
            d_tot += (float)dst[j]->second.histogram[i] / dst.size() / dst[j]->second.pixelcount;
        sum += sqrt(s_tot * d_tot);
    }
    
    return sum;
}
