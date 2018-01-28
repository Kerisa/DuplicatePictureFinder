#include <iostream>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <math.h>

#include "isodata.h"

#define iniClusters 100  //��ʼ��۵ĸ���

using namespace std;

//����6��ʹ�õĲ���
struct Args
{
    int expClusters;   //�����õ��ľ�����
    int thetaN;        //����������������
    int maxIts;        //����������
    int combL;         //ÿ�ε�������ϲ������������
    double thetaS;     //��׼ƫ�����
    double thetaC;     //�ϲ�����
}args;


//��Ҫ�ϲ���������۵���Ϣ������������۵�id�;���
struct MergeInfo
{
    int u, v;
    double d;    //���u���������v���ĵľ���
};

//����ȽϺ���
bool cmp(MergeInfo a, MergeInfo b)
{
    return a.d < b.d;
}

//��������֮�����
double dist(const Point & A, const Point & B)
{
    //assert(A.totle_count != 0 && B.totle_count != 0);
#if 0
    double A_sum = 0.0;
    double B_sum = 0.0;
    double totle = 0.0;
    for (int i = 0; i < Point::dimensions; ++i)
    {
        A_sum = (double)A.vec[i] / A.totle_count;
        B_sum = (double)B.vec[i] / B.totle_count;
        totle += sqrt(A_sum * B_sum);
    }
    assert(totle <= 1 && totle >= 0);
    return totle;
    //return sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
#endif

#if 0
    // ��һ��ŷ�Ͼ���
    double ret = 0.0;
    for (int i = 0; i < Point::dimensions; ++i)
    {
        double temp = A.vec[i] - B.vec[i];
        temp *= temp;
        ret += temp;
    }
    return sqrt(ret);
#endif

    // ���Ҿ���
    double numerator = 0.0;
    double length_A = 0.0;
    double length_B = 0.0;
    for (int i = 0; i < Point::dimensions; ++i)
    {
        numerator += A.vec[i] * B.vec[i];
        length_A += A.vec[i] * A.vec[i];
        length_B += B.vec[i] * B.vec[i];
    }
    assert(length_A > 0 && length_B > 0);
    double ret = numerator / sqrt(length_A) / sqrt(length_B);

    assert(ret >= -0.01 && ret <= 1.01);
    if (ret < 0)
        ret = 0;
    if (ret > 1)
        ret = 1;

    return 1 - ret;     // ֵԽСԽ�ӽ�
}

struct Cluster
{
    int nSamples{ 0 };          //������ĸ���
    double avgDist{ 0.0 };        //�����㵽�������ĵ�ƽ������
    Point center;          //��������
    Point sigma;           //���������ĵı�׼��
    vector<Point *> data;  //���������

                           //����þ�������ģ�������ľ�ֵ
    void calMean()
    {
        assert(nSamples == data.size());
        center.Reset();
        for (int d = 0; d < Point::dimensions; ++d)
        {
            for (int i = 0; i < nSamples; i++)
            {
                center.vec[d] += data[i]->vec[d];
                center.totle_count += data[i]->vec[d];
            }
            center.vec[d] /= nSamples;
        }
        center.totle_count /= nSamples;
    }

    //������������㵽�þ������ĵ�ƽ������
    void calDist()
    {
        avgDist = 0;
        for (int i = 0; i < nSamples; i++)
            avgDist += dist(*(data.at(i)), center);
        avgDist /= nSamples;
    }

    //�������������ĵı�׼��
    void calStErr()
    {
        assert(nSamples == data.size());
        sigma.Reset();
        for (int d = 0; d < Point::dimensions; ++d)
        {
            double temp = 0;
            for (int i = 0; i < nSamples; ++i)
            {
                double dis = data[i]->vec[d] - center.vec[d];
                temp += (dis * dis);
            }
            sigma.vec[d] = sqrt(temp / nSamples);
        }
    }
};

//���ò�����ֵ
void setArgs()
{
    args.expClusters = 75;
    args.thetaN = 1;
    args.maxIts = 150;
    args.combL = 10;
    args.thetaS = 60;
    args.thetaC = 0.01;
}

//Ѱ�ҵ�t���������������Ķ�Ӧ��id
int FindIdx(vector<Cluster> &c, Point &t)
{
    int nClusters = c.size();
    assert(nClusters >= 1);
    double ans = dist(c.at(0).center, t);
    int idx = 0;
    for (int i = 1; i < nClusters; i++)
    {
        double tmp = dist(c.at(i).center, t);
        if (ans > tmp)
        {
            idx = i;
            ans = tmp;
        }
    }
    return idx;
}

//���ַ�Ѱ�Ҿ���պ�С��thetaC��������۵�index
int FindPos(MergeInfo *info, int n, double thetaC)
{
    int l = 0;
    int r = n - 1;
    while (l <= r)
    {
        int mid = (l + r) >> 1;
        if (info[mid].d < thetaC)
        {
            l = mid + 1;
            if (l < n && info[l].d >= thetaC)
                return mid;
        }
        else
        {
            r = mid - 1;
            if (r >= 0 && info[r].d < thetaC)
                return r;
        }
    }
    if (info[n - 1].d < thetaC)
        return n - 1;
    else
        return -1;
}

void ISOData(Point p[], int n)
{
    cout << "ISOData is processing......." << endl;
    vector<Cluster> c;              //ÿ����۵�����
    const double split = 0.5;       //���ѳ���(0,1]
    int nClusters = n > iniClusters ? iniClusters : n;    //��ʼ����۸���

                                    //��ʼ��nClusters���࣬�����������
    for (int i = 0; i < nClusters; i++)
    {
        Cluster t;
        t.center = p[i];
        t.nSamples = 0;
        t.avgDist = 0;
        c.push_back(t);
    }

    int iter = 0;
    bool isLess = false;            //��־�Ƿ��������Ŀ����thetaN
    while (1)
    {
        printf("turn: %d, cluster: %d\n", iter + 1, c.size());

        //�����ÿһ������
        for (int i = 0; i < nClusters; i++)
        {
            c.at(i).nSamples = 0;
            c.at(i).data.clear();
        }

        //�������������ֵ���������������������
        for (int i = 0; i < n; i++)
        {
            int idx = FindIdx(c, p[i]);
            c.at(idx).data.push_back(&p[i]);
            c.at(idx).nSamples++;
        }

DeleteNext:
        int k = 0;                   //��¼������Ŀ����thetaN�����index
        for (int i = 0; i < nClusters; i++)
        {
            if (c.at(i).data.size() < args.thetaN)
            {
                isLess = true;       //˵�����������٣�����Ӧ��ɾ��
                k = i;
                break;
            }
        }

        //��������������ĿС��thetaN
        if (isLess)
        {
            nClusters--;
            Cluster t = c.at(k);
            vector<Cluster>::iterator pos = c.begin() + k;
            c.erase(pos);
            assert(nClusters == c.size());
            for (int i = 0; i < t.data.size(); i++)
            {
                int idx = FindIdx(c, *(t.data.at(i)));
                c.at(idx).data.push_back(t.data.at(i));
                c.at(idx).nSamples++;
            }
            isLess = false;
            goto DeleteNext;
        }

        for (int kk = 0; kk < nClusters; ++kk)
        {
            assert(!c[kk].data.empty());
        }

        //���¼����ֵ��������������ĵ�ƽ������
        for (int i = 0; i < nClusters; i++)
        {
            c.at(i).calMean();
            c.at(i).calDist();
        }

        //�����ܵ�ƽ������
        double totalAvgDist = 0;
        for (int i = 0; i < nClusters; i++)
            totalAvgDist += c.at(i).avgDist * c.at(i).nSamples;
        totalAvgDist /= n;

        if (iter >= args.maxIts)
            break;

        //���Ѳ���
        if (nClusters <= args.expClusters / 2)
        {
            // ��������ֵ��������������
            vector<pair<double, int>> maxsigma;
            for (int i = 0; i < nClusters; i++)
            {
                //�������ı�׼ƫ��
                c.at(i).calStErr();
                //��������׼���������
                pair<double, int> max_st = pair<double, int>(0.0, 0);
                for (int d = 0; d < Point::dimensions; ++d)
                {
                    if (c[i].sigma.vec[d] > max_st.first)
                    {
                        max_st.first = c[i].sigma.vec[d];
                        max_st.second = d;
                    }
                }
                maxsigma.push_back(max_st);
            }
            bool splitd = false;
            for (int i = 0; i < maxsigma.size(); i++)
            {
                if (maxsigma.at(i).first > args.thetaS)
                {
                    if ((c.at(i).avgDist > totalAvgDist && c.at(i).nSamples > 2 * (args.thetaN + 1)) || (nClusters < args.expClusters / 2))
                    {
                        splitd = true;

                        nClusters++;
                        Cluster newCtr;     //�µľ�������
                                            //��ȡ�µ�����
                        for (int d = 0; d < Point::dimensions; ++d)
                        {
                            newCtr.center.vec[d] = c[i].center.vec[d];
                        }
                        newCtr.center.vec[maxsigma[i].second] = c[i].center.vec[maxsigma[i].second] - split * maxsigma[i].first;
                        newCtr.center.totle_count = 0; 
                        for (int d = 0; d < Point::dimensions; ++d)
                        {
                            newCtr.center.totle_count += newCtr.center.vec[d];
                        }

                        //newCtr.center.x = c.at(i).center.x - split * c.at(i).sigma.x;
                        //newCtr.center.y = c.at(i).center.y - split * c.at(i).sigma.y;                        
                        c.push_back(newCtr);
                        //�ı��ϵ�����
                        c[i].center.vec[maxsigma[i].second] += split * maxsigma[i].first;
                        c[i].center.totle_count = 0;
                        for (int d = 0; d < Point::dimensions; ++d)
                        {
                            c[i].center.totle_count += c[i].center.vec[d];
                        }
                        //c.at(i).center.x = c.at(i).center.x + split * c.at(i).sigma.x;
                        //c.at(i).center.y = c.at(i).center.y + split * c.at(i).sigma.y;
                        
                        break;
                    }
                }
            }
            //if (splitd)
            //{
            //    ++iter;
            //    continue;
            //}
        }


        //�ϲ�����
        if (nClusters >= 2 * args.expClusters || (iter & 1) == 0)
        {
            int size = nClusters * (nClusters - 1);
            //��Ҫ�ϲ��ľ������
            int cnt = 0;
            MergeInfo *info = new MergeInfo[size];
            for (int i = 0; i < nClusters; i++)
            {
                for (int j = i + 1; j < nClusters; j++)
                {
                    info[cnt].u = i;
                    info[cnt].v = j;
                    info[cnt].d = dist(c.at(i).center, c.at(j).center);
                    cnt++;
                }
            }
            //��������
            sort(info, info + cnt, cmp);
            vector<MergeInfo *> __MergerInfo;
            for (int i = 0; i < size; ++i) {
                __MergerInfo.push_back(&info[i]);
            }
            //�ҳ�info�����о���պ�С��thetaC��index����ôindex��С�ĸ�Ӧ�úϲ�
            int iPos = FindPos(info, cnt, args.thetaC);

            //����ָʾ��λ�õ��������Ƿ��Ѿ��ϲ�
            bool *flag = new bool[nClusters];
            memset(flag, false, sizeof(bool) * nClusters);
            //���ڱ�Ǹ�λ�õ��������Ƿ��Ѿ��ϲ�ɾ��
            bool *del = new bool[nClusters];
            memset(del, false, sizeof(bool) * nClusters);
            //��¼�ϲ��Ĵ���
            int nTimes = 0;

            assert(nClusters == c.size());
            for (int i = 0; i <= iPos; i++)
            {
                int u = info[i].u;
                int v = info[i].v;
                //ȷ��ͬһ�����ֻ�ϲ�һ��
                if (!flag[u] && !flag[v])
                {
                    flag[u] = flag[v] = true;
                    //���һ�ε����кϲ���������combL����ֹͣ�ϲ�
                    if (nTimes > args.combL)
                        break;

                    nTimes++;
                    //����Ŀ�ٵ������ϲ�����Ŀ���������
                    if (c.at(u).nSamples < c.at(v).nSamples)
                    {
                        assert(!del[u]);
                        del[u] = true;
                        Cluster t = c.at(u);
                        assert(t.nSamples == t.data.size());
                        for (int j = 0; j < t.nSamples; j++)
                            c.at(v).data.push_back(t.data.at(j));

                        for (int d = 0; d < Point::dimensions; ++d)
                        {
                            c[v].center.vec[d] = c[v].center.vec[d] * c[v].nSamples + t.nSamples * t.center.vec[d];
                        }
                        //c.at(v).center.x = c.at(v).center.x * c.at(v).nSamples + t.nSamples * t.center.x;
                        //c.at(v).center.y = c.at(v).center.y * c.at(v).nSamples + t.nSamples * t.center.y;
                        c.at(v).nSamples += t.nSamples;
                        for (int d = 0; d < Point::dimensions; ++d)
                        {
                            c[v].center.vec[d] /= c[v].nSamples;
                        }
                        //c.at(v).center.x /= c.at(v).nSamples;
                        //c.at(v).center.y /= c.at(v).nSamples;
                    }
                    else
                    {
                        assert(!del[v]);
                        del[v] = true;
                        Cluster t = c.at(v);
                        assert(t.nSamples == t.data.size());
                        for (int j = 0; j < t.nSamples; j++)
                            c.at(u).data.push_back(t.data.at(j));

                        for (int d = 0; d < Point::dimensions; ++d)
                        {
                            c[u].center.vec[d] = c[u].center.vec[d] * c[u].nSamples + t.nSamples * t.center.vec[d];
                        }
                        //c.at(u).center.x = c.at(u).center.x * c.at(u).nSamples + t.nSamples * t.center.x;
                        //c.at(u).center.y = c.at(u).center.y * c.at(u).nSamples + t.nSamples * t.center.y;
                        c.at(u).nSamples += t.nSamples;
                        for (int d = 0; d < Point::dimensions; ++d)
                        {
                            c[u].center.vec[d] /= c[u].nSamples;
                        }
                        //c.at(u).center.x /= c.at(u).nSamples;
                        //c.at(u).center.y /= c.at(u).nSamples;
                    }
                }
            }

            //ɾ���ϲ���ľ���
            vector<Cluster>::iterator id = c.begin();
            for (int i = 0; i < nClusters; i++)
            {
                if (del[i])
                    id = c.erase(id);
                else
                    id++;
            }

            //�ϲ����ٴξ�ɾ�����ٸ�
            nClusters -= nTimes;
            assert(nClusters == c.size());
            delete[] info;
            delete[] flag;
            delete[] del;
            info = NULL;
            flag = NULL;
            del = NULL;
        }

        if (iter >= args.maxIts)
            break;
        iter++;
    }
    assert(nClusters == c.size());
    //Print(c);
}

int isodata_entrance(Point *p, int n)
{
    //int n = 0;
    //scanf_s("%d", &n);
    //Point *p = new Point[n];

    //getData(p, n);
    setArgs();
    ISOData(p, n);

    //delete[] p;
    //p = NULL;

    return 0;
}