#pragma once

#include <map>
#include <string>
#include <vector>

struct ImageInfo;

struct FeatureData
{
    std::wstring filename;
    int pixelcount;          // ������ά��, = 0x100 / mDivideRegion
    int group;              // ����󱻹��ൽ FeatureVector::mGroup[group] ��
    int *histogram;
    FeatureData() : histogram(nullptr), group(-1) { }
    FeatureData(FeatureData && fd)
    {
        pixelcount = fd.pixelcount;
        filename.assign(fd.filename);
        histogram = fd.histogram;
        group = fd.group;

        fd.filename.clear();
        fd.histogram = nullptr;
        fd.pixelcount = 0;
        fd.group = -1;
    }
    ~FeatureData()
    {
        if (histogram)
            delete[] histogram;
    }
private:
    FeatureData(const FeatureData &) { }
};

class FeatureVector
{
public:
    typedef std::map<unsigned int, FeatureData> SingleDataMap;
    typedef void(*fn_image_cmp_result)(const std::vector<std::vector<SingleDataMap::iterator>> &mGroup);

private:
    struct __helpdata
    {
        const float invalid_value = -1;
        int element_count;
        int filled_index;       // �Ѹ��µ�������
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
    FeatureVector();
    bool Initialize(int _iterations = 1, int _DivideRegion = 16);
    void Clear();
    bool AddPicture(const wchar_t * filename, const ImageInfo *pinfo);
    bool DivideGroup(fn_image_cmp_result callback);
    
    // ��һ��svd
    void svd();

private:
    inline int min(int t, int v) { return t < v ? t : v; }

    // ���Ͼ���
    float Calc(const FeatureData &src, const FeatureData &dst);
    float CalcGroup(std::vector<SingleDataMap::iterator> &src, std::vector<SingleDataMap::iterator> &dst, __helpdata *phsrc = nullptr, __helpdata *phdst = nullptr);
    float CalcGroup2(std::vector<SingleDataMap::iterator> &src, std::vector<SingleDataMap::iterator> &dst, __helpdata *phsrc = nullptr, __helpdata *phdst = nullptr);

private:
    int mDivideRegion;                  // ͼ����ɫ���־���, ÿmcDivideRegion��������Ϊһ������
                                        // ֵԽ�����ж�Խ�ֲ�
    int mDimension;                     // = 0x100 / mcDivideRegion
    const float mcThreshold = 0.95f;    // �ж�ͼ�����Ƶ���ֵ
    int mIterations;                    // ��������Ĵ���

    SingleDataMap mData;
    std::vector<std::vector<SingleDataMap::iterator>> mGroup;

    static bool CrcInitialized;
};