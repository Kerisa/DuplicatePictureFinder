#pragma once

#include <map>
#include <string>
#include <vector>

#include "types.h"

namespace Alisa
{
    class Image;

    class FeatureData
    {
    public:
        FeatureData() = default;
        FeatureData(FeatureData && fd);
        ~FeatureData();

        const string_t & GetFilename() const { return Filename; }
        int GetPixelCount() const { return PixelCount; }
        bool GetHistogram(const int **ptr) const;
        bool BuildHistogram(const Image & img, const string_t & filename);

    private:
        static constexpr int Dimension    = 16;     // 每个通道划分为 16 段
        static constexpr int DivideRegion = 16;     // 划分后每个通道中每一段含有的像素分量为 16
                                                    // DivideRegion * Dimension == 256
    public:
        static constexpr int HistogramLength = Dimension * Dimension * Dimension;

    private:
        string_t        Filename;
        int             PixelCount{ 0 };
        int             GroupIdx{ -1 };              // 计算后被归类到 ImageFeatureVector::mGroup[GroupIdx] 组
        int *           Histogram{ nullptr };

    private:
        FeatureData(const FeatureData &) = delete;
        FeatureData & operator=(const FeatureData &) = delete;
    };

    class ImageFeatureVector
    {
    public:
        typedef std::map<unsigned int, FeatureData> SingleDataMap;
        enum ProcessState{
            STATE_NOT_START,
            STATE_PROCESSING,
            STATE_FINISH,
        };

    private:
        struct __helpdata
        {
            const float invalid_value = -1;
            int element_count;
            int filled_index;       // 已更新的索引号
            float *pvalues;
            __helpdata() : element_count(0), filled_index(0), pvalues(nullptr) { }
            void Init(int size)
            {
                if (!pvalues)
                {
                    pvalues = new float[size];
                    for (int i = 0; i < size; ++i)
                        pvalues[i] = invalid_value;
                }

            }
            ~__helpdata()
            {
                if (pvalues) delete[] pvalues;
            }
        private:
            __helpdata(const __helpdata &);
        };

    public:
        ImageFeatureVector();
        bool Initialize(int _iterations = 1, int _DivideRegion = 16);
        void Clear();
        bool AddPicture(const wchar_t * filename, const Image & img);
        bool DivideGroup();
        std::vector<std::vector<string_t>> GetGroupResult() const;

        // 试一下svd
        void svd();
        bool IsoData();

    private:
        inline int min(int t, int v) { return t < v ? t : v; }

        // 巴氏距离
#if 0
        float Calc(const FeatureData &src, const FeatureData &dst);
#endif
        float CalcGroup(std::vector<SingleDataMap::iterator> &src, std::vector<SingleDataMap::iterator> &dst, __helpdata *phsrc = nullptr, __helpdata *phdst = nullptr);
#if 0
        float CalcGroup2(std::vector<SingleDataMap::iterator> &src, std::vector<SingleDataMap::iterator> &dst, __helpdata *phsrc = nullptr, __helpdata *phdst = nullptr);
#endif

    private:
        int mDivideRegion;                  // 图像颜色划分精度, 每mcDivideRegion个像素作为一个区间
                                            // 值越大则判断越粗糙
        int mDimension;                     // = 0x100 / mcDivideRegion
        const float mcThreshold = 0.95f;    // 判断图像相似的阈值
        int mIterations;                    // 计算迭代的次数

        ProcessState mProcessState;

        SingleDataMap mData;
        std::vector<std::vector<SingleDataMap::iterator>> mGroup;

        static bool CrcInitialized;
    };
}
