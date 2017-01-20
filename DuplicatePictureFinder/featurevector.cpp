
#include <assert.h>
#include <locale.h>
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

bool FeatureVector::Initialize(int _iterations, int _DivideRegion)
{
    Clear();
    mIterations = _iterations;
    mDivideRegion = _DivideRegion;
    mDimension = (0x100 + _DivideRegion - 1) / _DivideRegion;
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

bool FeatureVector::AddPicture(const wchar_t * filename, const ImageInfo * pinfo)
{
    assert(mDivideRegion > 0);

    if (!filename || !pinfo || 
        pinfo->height <= 0 || pinfo->width <= 0 || pinfo->component <= 0 ||
        pinfo->ppixels == nullptr)
        return false;

    unsigned int crc = CRC32_4((const unsigned char*)filename, 0, wcslen(filename) * sizeof(wchar_t));
    assert(mData.find(crc) == mData.end());
    auto res = mData.insert(std::make_pair(crc, FeatureData()));
    FeatureData & data = res.first->second;
    data.filename.assign(filename);
    data.pixelcount = pinfo->width * pinfo->height;
    
    data.histogram = new int[mDimension * mDimension * mDimension];
    memset(data.histogram, 0, mDimension * mDimension * mDimension * sizeof(int));

    if (pinfo->component == 3 || pinfo->component == 4)
        for (int i = 0; i < pinfo->width * pinfo->height * pinfo->component; i += pinfo->component)
        {
            unsigned char R = pinfo->ppixels[i];
            unsigned char G = pinfo->ppixels[i + 1];
            unsigned char B = pinfo->ppixels[i + 2];

            ++data.histogram[R / mDivideRegion * mDimension * mDimension +
                             G / mDivideRegion * mDimension +
                             B / mDivideRegion];
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

    const int size = mDimension * mDimension * mDimension;
    __helpdata *hd = new __helpdata[mGroup.size()];
    for (int i = 0; i < mGroup.size(); ++i)
        hd[i].Init(size);

    while (mIterations--)
    {
        for (int i = 0; i < mGroup.size(); ++i)
        {
            for (int j = i + 1; j < mGroup.size(); ++j)
            {
                float distance = CalcGroup(mGroup[i], mGroup[j], &hd[i], &hd[j]);
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

    delete[] hd;

#ifdef _DEBUG
    setlocale(LC_ALL, "");
    FILE *debug_out;
    fopen_s(&debug_out, "debug_out.log", "wb");
    int gcnt = 0;
    for (int i = 0; i < mGroup.size(); ++i)
    {
        if (mGroup[i].empty())
            continue;

        fprintf(debug_out, "Group: %d, element: %d\r\n", gcnt, mGroup[i].size());
        for (auto it = mGroup[i].begin(); it != mGroup[i].end(); ++it)
            fprintf(debug_out, "    %ws\r\n", (*it)->second.filename.c_str());
        fprintf(debug_out, "\r\n");

        ++gcnt;
    }
    fclose(debug_out);
    setlocale(LC_ALL, "C");
#endif

    return true;
}

float FeatureVector::Calc(const FeatureData &src, const FeatureData &dst)
{
    float sum = 0.0f;
    int times = mDimension * mDimension * mDimension;  // 向量的维数

    for (int i = 0; i < times; ++i)
        sum += sqrt(((float)src.histogram[i] / times) * ((float)dst.histogram[i] / times));
    
#ifdef _DEBUG
    fprintf(stderr, "%ws(%d) - %ws(%d) : %.5f",
        src.filename.c_str(), src.group,
        dst.filename.c_str(), dst.group, sum);
#endif

    return sum;
}

float FeatureVector::CalcGroup(
    std::vector<SingleDataMap::iterator> &src,
    std::vector<SingleDataMap::iterator> &dst,
    __helpdata *phsrc,
    __helpdata *phdst
)
{
    float sum = 0.0f;
    int times = mDimension * mDimension * mDimension;  // 向量的维数

    if (src.empty() || dst.empty())
        return sum;

    for (int i = 0; i < times; ++i)
    {
        float s_tot = 0.0f, d_tot = 0.0f;
        if (phsrc && phsrc->element_count == src.size()
            && phsrc->pvalues[i] != phsrc->invalid_value)    // 数据有效
            s_tot = phsrc->pvalues[i];
        else
        {
            for (int j = 0; j < src.size(); ++j)
                s_tot += (float)src[j]->second.histogram[i] / src.size() / src[j]->second.pixelcount;
            if (phsrc)
            {
                // 一旦组里添加了新的文件，则缓存全部失效
                if (phsrc->element_count != src.size())
                {
                    // 这种情况只可能在计算第一个块时出现
                    assert(i == 0);
                    // 更新缓存数据
                    phsrc->element_count = src.size();
                }
                // 同时清除下一个，以便继续更新
                if (i + 1 < times - 1)
                    phsrc->pvalues[i + 1] = phsrc->invalid_value;
                // 更新缓存数据
                phsrc->pvalues[i] = s_tot;
            }
        }

        if (phdst && phdst->element_count == dst.size()
            && phdst->pvalues[i] != phsrc->invalid_value)
            d_tot = phdst->pvalues[i];
        else
        {
            for (int j = 0; j < dst.size(); ++j)
                d_tot += (float)dst[j]->second.histogram[i] / dst.size() / dst[j]->second.pixelcount;
            if (phdst)
            {
                if (phdst->element_count != dst.size())
                {
                    assert(i == 0);
                    phdst->element_count = dst.size();
                }
                phdst->pvalues[i] = d_tot;
                if (i + 1 < times - 1)
                    phdst->pvalues[i + 1] = phdst->invalid_value;
            }
        }
        sum += sqrt(s_tot * d_tot);
    }
    
    return sum;
}

float FeatureVector::CalcGroup2(
    std::vector<SingleDataMap::iterator>& src,
    std::vector<SingleDataMap::iterator>& dst
)
{
    // 只用第一个元素（图像）进行计算
    // 更精确的分组...？
    return 0.0f;
}
