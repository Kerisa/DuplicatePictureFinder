#include "qpicthread.h"
#include "mainwindow.h"
#include "OpenFiles.h"
#include "picture.h"
#include "featurevector.h"

#include <array>

#define _U(str) QString::fromWCharArray(L##str)

struct SubThreadData
{
    std::vector<const std::wstring *>::const_iterator  fileStart;
    std::vector<const std::wstring *>::const_iterator  fileEnd;
    std::vector<Alisa::ImageInfo>::iterator     imgStart;
    std::vector<Alisa::ImageInfo>::iterator     imgEnd;
    Alisa::ImageFeatureVector *                 fv;
    bool *                                      continueProc;
    int                                         procCnt;
    int                                         failedCnt;
};

class ReadFileSubThread : public QThread
{
public:
    ReadFileSubThread(SubThreadData *data) : Data(data) { }

    virtual void run()
    {
        auto imgIt = Data->imgStart;
        for (auto it = Data->fileStart; it != Data->fileEnd && *Data->continueProc; ++Data->procCnt, ++it, ++imgIt)
        {
            Alisa::Image image;
            if (!image.Open(**it))
            {
                ++Data->failedCnt;
                Q_ASSERT(0);
                continue;
            }

            *imgIt = image.GetImageInfo();
            Data->fv->AddPicture((*it)->c_str(), image);

        }
        Q_ASSERT(!*Data->continueProc || imgIt == Data->imgEnd);
        exit();
    }

private:
    SubThreadData *Data;
};


QPicThread::QPicThread(MainWindow *mainWnd)
{
    continueRun = true;
    Threshold = 0.95f;
    MainWnd = mainWnd;

    auto b = connect(this, SIGNAL(PictureProcessFinish()), MainWnd, SLOT(OnPictureProcessFinish()));
    Q_ASSERT(b);
    b = connect(this, SIGNAL(PictureProcessStepMsg(float, const QString &)), MainWnd, SLOT(OnPictureProcessStep(float, const QString &)));
    Q_ASSERT(b);
}

void QPicThread::run()
{
    continueRun = true;

    std::vector<std::wstring> path, out;
    for (auto & qs : Path)
        path.push_back(qs.toStdWString());
    GetSubFileList(path, out);

    Alisa::ImageFeatureVector fv;
    fv.Initialize(Threshold);


    emit PictureProcessStepMsg(0, _U("正在扫描文件..."));

    // 筛除不处理的文件
    std::vector<const std::wstring *> out2;
    for (size_t i = 0; i < out.size() && continueRun; ++i)
    {
        QString extension = QString::fromStdWString(out[i].substr(out[i].find_last_of('.') + 1));
        if (extension.compare("png", Qt::CaseInsensitive) &&
            extension.compare("bmp", Qt::CaseInsensitive) &&
            extension.compare("jpg", Qt::CaseInsensitive))
                continue;

        out2.push_back(&out[i]);
    }

    // 存放图像基本信息，与 out2 一一对应
    std::vector<Alisa::ImageInfo> imagesInfo;
    imagesInfo.resize(out2.size());

    // 多线程读取
    const int threadCounts = 5;
    QVector<QPair<ReadFileSubThread*, SubThreadData*>> threads;
    int partCount = (out2.size() + threadCounts - 1) / threadCounts;
    for (int i = 0; i < threadCounts; ++i)
    {
        auto d = new SubThreadData;
        d->continueProc = &continueRun;
        d->failedCnt =0;
        d->procCnt = 0;
        d->fv = &fv;
        d->fileStart = out2.begin() + i * partCount;
        d->fileEnd   = out2.begin() + qMin<int>((i + 1) * partCount, out2.size());
        d->imgStart  = imagesInfo.begin() + i * partCount;
        d->imgEnd    = imagesInfo.begin() + qMin<int>((i + 1) * partCount, out2.size());

        auto th = new ReadFileSubThread(d);
        threads.push_back(QPair<ReadFileSubThread*, SubThreadData*>(th, d));
        th->start();
    }

    // 等待处理并更新状态栏进度
    while (1)
    {
        int count = 0;
        for (int i = 0; i < threads.size(); ++i)
        {
            count += threads[i].second->procCnt;
        }
        emit PictureProcessStepMsg(0.7f * count / out2.size(), _U("正在扫描文件..."));
        msleep(1000);
        if (!continueRun || count >= out2.size())
        {
            for (auto &th : threads)
            {
                th.first->wait();
            }
            break;
        }
    }

    int readFailed = 0;
    for (auto & th : threads)
    {
        readFailed += th.second->failedCnt;
    }
    if (readFailed > 0)
    {
        Q_ASSERT(0);
    }
    for (auto & th : threads)
    {
        delete th.first;
        delete th.second;
    }


    if (!continueRun)
    {
        emit PictureProcessStepMsg(0, _U("处理中止"));
        exit();
        return;
    }

    emit PictureProcessStepMsg(0.7f, _U("正在比较..."));

    fv.DivideGroup();

    if (!continueRun)
    {
        emit PictureProcessStepMsg(0, _U("处理中止"));
        exit();
        return;
    }

    emit PictureProcessStepMsg(0.95f, _U("正在生成结果..."));

    std::vector<std::vector<TreeViewImageInfo>> groups;
    auto _result = fv.GetGroupResult();
    for (auto & g : _result)
    {
        groups.push_back(std::vector<TreeViewImageInfo>());
        auto & curGroup = groups.back();
        for (auto & fileName : g)
        {
            TreeViewImageInfo info;
            info.fileName = QString::fromStdWString(fileName);
            for (size_t k = 0; k < out2.size(); ++k)
            {
                if (!out2[k]->compare(fileName))
                {
                    auto & imageBaseInfo = imagesInfo[k];
                    info.height = imageBaseInfo.Height;
                    info.width = imageBaseInfo.Width;
                    info.component = imageBaseInfo.Component;
                    break;
                }
            }
            info.fileInfo.setFile(QString::fromStdWString(fileName));
            Q_ASSERT(info.fileInfo.exists());
            curGroup.push_back(info);
        }
    }
    MainWnd->SetGroupResult(groups);


    emit PictureProcessStepMsg(1, _U("查找完成"));
    emit PictureProcessFinish();

    exit();
}

void QPicThread::SetPath(const QStringList &path, float threshold)
{
    Path = path;
    Threshold = threshold;
}

bool QPicThread::Abort()
{
    if (!isRunning())
        return true;

    continueRun = false;
    return wait(5000);
}
