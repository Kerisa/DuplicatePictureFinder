
#include <assert.h>
#include <locale.h>
#include "featurevector.h"
#include "picture.h"
#include "crc.h"
#include "Utility.h"


Alisa::FeatureData::FeatureData(FeatureData && fd)
{
    PixelCount = fd.PixelCount;
    Filename.assign(fd.Filename);
    Histogram = fd.Histogram;
    GroupIdx = fd.GroupIdx;

    fd.Filename.clear();
    fd.Histogram = nullptr;
    fd.PixelCount = 0;
    fd.GroupIdx = -1;
}

Alisa::FeatureData::~FeatureData()
{
    if (Histogram)
        delete[] Histogram;
}

bool Alisa::FeatureData::GetHistogram(const int ** ptr) const
{
    *ptr = Histogram;
    return true;
}

bool Alisa::FeatureData::BuildHistogram(const Image & img, const string_t & filename)
{
    Filename = filename;
    PixelCount = img.GetImageInfo().Height * img.GetImageInfo().Width;
    assert(!Histogram);

    Histogram = new int[HistogramLength];
    memset(Histogram, 0, HistogramLength * sizeof(int));

    if (img.GetImageInfo().Component == PixelType_RGB || img.GetImageInfo().Component == PixelType_RGBA)
    {
        auto pixels = img.GetPixelsGroup();
        for (size_t h = 0; h < pixels.size(); ++h)
        {
            for (size_t w = 0; w < pixels[h].size(); ++w)
            {
                auto & p = pixels[h][w];
                ++Histogram[p.R / DivideRegion * Dimension * Dimension +
                    p.G / DivideRegion * Dimension +
                    p.B / DivideRegion];
            }
        }
        //img.WalkPixels([this](int row, int col, const Pixel & pixel) {
        //    ++Histogram[pixel.R / DivideRegion * Dimension * Dimension +
        //                pixel.G / DivideRegion * Dimension +
        //                pixel.B / DivideRegion];
        //});
        return true;
    }
    else
    {
        Utility::SafeDeleteArray(&Histogram);
        assert(0);
        return false;
    }
}


////////////////////////////////////////////////////////////////////////////////



bool Alisa::ImageFeatureVector::CrcInitialized = false;

Alisa::ImageFeatureVector::ImageFeatureVector()
{
    if (!CrcInitialized)
    {
        create_crc_table();
        CrcInitialized = true;
    }
}

bool Alisa::ImageFeatureVector::Initialize(float threshold, int iterations)
{
    Clear();
    mIterations = iterations;
    mThreshold = threshold;
    mProcessState = STATE_NOT_START;
    return true;
}

void Alisa::ImageFeatureVector::Clear()
{
    mProcessState = STATE_NOT_START;

    mIterations = 0;
    mData.clear();
    std::map<unsigned int, FeatureData>().swap(mData);
    mGroup.clear();
    std::vector<std::vector<SingleDataMap::iterator>>().swap(mGroup);
}

bool Alisa::ImageFeatureVector::AddPicture(const wchar_t * filename, const Image & img)
{
    assert(filename);
    unsigned int crc = CRC32_4((const unsigned char*)filename, 0, wcslen(filename) * sizeof(wchar_t));

    mDataLock.lock();
    assert(mData.find(crc) == mData.end());
    auto res = mData.insert(std::make_pair(crc, FeatureData()));
    FeatureData & data = res.first->second;
    mDataLock.unlock();

    if (!data.BuildHistogram(img, filename))
    {
        mDataLock.lock();
        mData.erase(crc);
        mDataLock.unlock();
        assert(0);
        return false;
    }

    // 添加图像会影响结果
    mProcessState = STATE_NOT_START;
    return true;
}

bool Alisa::ImageFeatureVector::DivideGroup()
{
    assert(mIterations > 0);
    mProcessState = STATE_PROCESSING;

    // 最初每张图像各成一组
    for (auto it = mData.begin(); it != mData.end(); ++it)
    {
        std::vector<std::map<unsigned int, FeatureData>::iterator> tmp;
        tmp.push_back(it);
        mGroup.push_back(tmp);
    }

    __helpdata *hd = new __helpdata[mGroup.size()];
    for (size_t i = 0; i < mGroup.size(); ++i)
        hd[i].Init(FeatureData::HistogramLength);

    while (mIterations--)
    {
        for (size_t i = 0; i < mGroup.size(); ++i)
        {
            for (size_t j = i + 1; j < mGroup.size(); ++j)
            {
                float distance = CalcGroup(mGroup[i], mGroup[j], &hd[i], &hd[j]);
                //float distance = CalcGroup2(mGroup[i], mGroup[j], &hd[i], &hd[j]);

//#if 0
#ifdef _DEBUG
                if (!mGroup[i].empty() && !mGroup[j].empty())
                    fprintf(stderr, "Group[%d](%d) to Group[%d](%d): Distance=%0.6f\r\n",
                        i, mGroup[i].size(), j, mGroup[j].size(), distance);
#endif

                if (distance > mThreshold)
                {
                    // 相似
                    mGroup[i].insert(mGroup[i].end(), mGroup[j].begin(), mGroup[j].end());
                    mGroup[j].clear();
                }
            }
        }
    }

    delete[] hd;

    mProcessState = STATE_FINISH;

    auto _result = GetGroupResult();

#if 1
//#ifdef _DEBUG
    setlocale(LC_ALL, "");
    FILE *debug_out = NULL;
    errno_t err = fopen_s(&debug_out, "debug_out.log", "wb");
    if (!err)
    {
        int gcnt = 0;
        for (size_t i = 0; i < mGroup.size(); ++i)
        {
            if (mGroup[i].size() <= 1)
                continue;

            fprintf(debug_out, "Group: %d, element: %d\r\n", gcnt, mGroup[i].size());
            for (auto it = mGroup[i].begin(); it != mGroup[i].end(); ++it)
                fprintf(debug_out, "    %ws\r\n", (*it)->second.GetFilename().c_str());
            fprintf(debug_out, "\r\n");

            ++gcnt;
        }
        fclose(debug_out);
    }
    setlocale(LC_ALL, "C");
#endif

    return true;
}

std::vector<std::vector<string_t>> Alisa::ImageFeatureVector::GetGroupResult() const
{
    std::vector<std::vector<string_t>> result;
    if (mProcessState != STATE_FINISH)
        return result;

    for (auto & g : mGroup)
    {
        if (g.size() <= 1)  // 没有重复
            continue;

        result.push_back(std::vector<string_t>());
        auto & curGroup = result.back();
        for (auto & f : g)
        {
            curGroup.push_back(f->second.GetFilename());
        }
    }
    return result;
}

float Alisa::ImageFeatureVector::CalcGroup(
    std::vector<SingleDataMap::iterator> &src,
    std::vector<SingleDataMap::iterator> &dst,
    __helpdata *phsrc,
    __helpdata *phdst
)
{
    float sum = 0.0f;
    const int times = FeatureData::HistogramLength;  // 向量的维数

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
            for (size_t j = 0; j < src.size(); ++j)
            {
                const int *ptr = nullptr;
                auto ret = src[j]->second.GetHistogram(&ptr);
                assert(ret);
                s_tot += (float)ptr[i] / src.size() / src[j]->second.GetPixelCount();
            }
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
            for (size_t j = 0; j < dst.size(); ++j)
            {
                const int *ptr = nullptr;
                auto ret = dst[j]->second.GetHistogram(&ptr);
                assert(ret);
                d_tot += (float)ptr[i] / dst.size() / dst[j]->second.GetPixelCount();
            }
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
