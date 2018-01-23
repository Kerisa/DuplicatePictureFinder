#pragma once


//定义二维点，这里假设是二维的特征，当然可以推广到多维
struct Point
{
    static constexpr int dimensions = 16 * 16 * 16;

    Point() { Reset(); }
    void Set(int *ptr, const wchar_t *_name)
    {
        wcscpy_s(name, _countof(name), _name);
        totle_count = 0;
        for (int i = 0; i < dimensions; ++i)
        {
            totle_count += ptr[i];
            vec[i] = ptr[i];
        }

        //double scale = 1000.0;
        //// Normalization
        //for (int i = 0; i < dimensions; ++i)
        //{
        //    vec[i] = (double)scale * ptr[i] / totle_count;
        //}
    }
    void Reset() { totle_count = 0; name[0] = 0; memset(vec, 0, dimensions * sizeof(double)); }

    double vec[dimensions];
    int totle_count{ 0 };
    wchar_t name[1024];
};

int isodata_entrance(Point *p, int n);