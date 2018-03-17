
#include <QApplication>
#include <array>
#include "qpicthread.h"
#include "mainwindow.h"
#include "OpenFiles.h"
#include "picture.h"
#include "featurevector.h"

#include <windows.h>

#define _U(str) QString::fromWCharArray(L##str)

struct SubThreadData
{
    std::vector<std::wstring>::const_iterator   fileStart;
    std::vector<std::wstring>::const_iterator   fileEnd;
    std::vector<Alisa::ImageInfo>::iterator     imgStart;
    std::vector<Alisa::ImageInfo>::iterator     imgEnd;
    Alisa::ImageFeatureVector *                 fv{ nullptr };
    Alisa::FeatureDataRecord *                  cache{nullptr};
    bool *                                      continueProc{ nullptr };        // 是否中断处理
    int                                         readFileCount{ 0 };             // 已读取的数量
    std::vector<const std::wstring *>           failedFile;                     // 读取失败的文件名
};

class ReadFileSubThread : public QThread
{
public:
    ReadFileSubThread(SubThreadData *data) : Data(data) { }

    virtual void run()
    {
        auto imgIt = Data->imgStart;
        for (auto it = Data->fileStart; it != Data->fileEnd && *Data->continueProc; ++Data->readFileCount, ++it, ++imgIt)
        {
            if (!Data->cache->LoadCacheHistogram(*it, *Data->fv))
            {
                Alisa::Image image;
                if (!image.Open(*it))
                {
                    Data->failedFile.push_back(&*it);
                    continue;
                }
                *imgIt = image.GetImageInfo();
                Data->fv->AddPicture((it)->c_str(), image);
            }
            else
            {
                *imgIt = Alisa::Image::GetImageInfo(*it);
            }
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

int QPicThread::GetCPUCoreNum()
{
#if defined(WIN32)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
#elif defined(LINUX) || defined(SOLARIS) || defined(AIX)
    return get_nprocs();   //GNU fuction
#else
  #error unkonwn os
#endif
}

QString QPicThread::GetCacheFilePath()
{
    return QCoreApplication::applicationDirPath() + "\\cache.dat";
}

std::vector<std::wstring> QPicThread::FilterFiles(const std::vector<std::wstring> &in) const
{
    std::vector<std::wstring> fileGroup;
    for (size_t i = 0; i < in.size() && continueRun; ++i)
    {
        QString extension = QString::fromStdWString(in[i].substr(in[i].find_last_of('.') + 1));
        if (extension.compare("png", Qt::CaseInsensitive) &&
            extension.compare("bmp", Qt::CaseInsensitive) &&
            extension.compare("jpg", Qt::CaseInsensitive) &&
            extension.compare("jpeg", Qt::CaseInsensitive))
                continue;

        fileGroup.push_back(in[i]);
    }
    return fileGroup;
}

std::vector<QString> QPicThread::StartReadThread(
        std::vector<std::wstring> & fileGroup,
        std::vector<Alisa::ImageInfo> & imagesInfo,
        Alisa::ImageFeatureVector & fv,
        Alisa::FeatureDataRecord & fdr
        )
{
    // 多线程读取
    const int threadCounts = qMax(GetCPUCoreNum() - 1, 1);
    QVector<QPair<ReadFileSubThread*, SubThreadData*>> threads;
    int partCount = (fileGroup.size() + threadCounts - 1) / threadCounts;
    for (int i = 0; i < threadCounts; ++i)
    {
        auto d = new SubThreadData;
        d->continueProc  = &continueRun;
        d->readFileCount = 0;
        d->fv            = &fv;
        d->fileStart     = fileGroup.begin() + i * partCount;
        d->fileEnd       = fileGroup.begin() + qMin<int>((i + 1) * partCount, fileGroup.size());
        d->imgStart      = imagesInfo.begin() + i * partCount;
        d->imgEnd        = imagesInfo.begin() + qMin<int>((i + 1) * partCount, fileGroup.size());
        d->cache         = &fdr;

        auto th = new ReadFileSubThread(d);
        threads.push_back(QPair<ReadFileSubThread*, SubThreadData*>(th, d));
        th->start();
    }

    // 等待读取完成并更新状态栏进度
    while (1)
    {
        int count = 0;
        for (int i = 0; i < threads.size(); ++i)
        {
            count += threads[i].second->readFileCount;
        }
        emit PictureProcessStepMsg(0.7f * count / fileGroup.size(), _U("正在扫描文件..."));
        msleep(1000);
        if (!continueRun || count >= fileGroup.size())
        {
            // 结束等待
            for (auto &th : threads)
            {
                th.first->wait();
            }
            break;
        }
    }

    // 收集读取失败的文件
    std::vector<QString> readFailedFile;
    for (auto & th : threads)
    {
        for (auto & str : th.second->failedFile)
        {
            readFailedFile.push_back(QString::fromStdWString(*str));
        }
        delete th.first;
        delete th.second;
    }

    return readFailedFile;
}

void QPicThread::GenerateResult(
        const Alisa::ImageFeatureVector & fv,
        const std::vector<std::wstring> & fileGroup,
        const std::vector<Alisa::ImageInfo> & imagesInfo,
        const std::vector<QString> & readFailedFile
        ) const
{
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
            for (size_t k = 0; k < fileGroup.size(); ++k)
            {
                if (!fileGroup[k].compare(fileName))
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
    MainWnd->SetGroupResult(groups, readFailedFile);
}

void QPicThread::run()
{
    continueRun = true;    
    emit PictureProcessStepMsg(0, _U("正在扫描文件..."));

    std::vector<std::wstring> path, out;
    for (auto & qs : Path)
    {
        path.push_back(qs.toStdWString());
    }
    GetSubFileList(path, out);

    // 筛除不处理的文件
    std::vector<std::wstring> fileGroup = FilterFiles(out);

    // 存放图像基本信息，与 fileGroup 一一对应
    std::vector<Alisa::ImageInfo> imagesInfo;
    imagesInfo.resize(fileGroup.size());
    Alisa::ImageFeatureVector fv;
    fv.Initialize(Threshold);
    Alisa::FeatureDataRecord fdr;
    bool cacheFileExist = fdr.OpenExist(GetCacheFilePath().toStdWString());
    std::vector<QString> readFailedFile = StartReadThread(fileGroup, imagesInfo, fv, fdr);

    // 更新缓存
    if (!cacheFileExist)
    {
        fdr.Create(GetCacheFilePath().toStdWString());
    }
    fdr.SaveAllHistogramToFile(fv);
    fdr.Close();

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
    GenerateResult(fv, fileGroup, imagesInfo, readFailedFile);

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
