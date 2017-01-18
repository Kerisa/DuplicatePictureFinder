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

public:
    FeatureVector();
    bool Initialize(int _iterations = 1, int _DivideRegion = 16);
    void Clear();
    bool AddPicture(const wchar_t * filename, const ImageInfo *pinfo);
    bool DivideGroup(fn_image_cmp_result callback);

private:
    float Calc(const FeatureData &src, const FeatureData &dst);
    float CalcGroup(std::vector<SingleDataMap::iterator> &src, std::vector<SingleDataMap::iterator> &dst);

private:
    const int mcDivideRegion = 16;      // ͼ����ɫ���־���
    const float mcThreshold = 0.95f;    // �ж�ͼ�����Ƶ���ֵ
    int mIterations;                    // ��������Ĵ���

    SingleDataMap mData;
    std::vector<std::vector<SingleDataMap::iterator>> mGroup;

    static bool CrcInitialized;
};