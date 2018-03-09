#include "qpicthread.h"
#include "mainwindow.h"
#include "OpenFiles.h"
#include "picture.h"
#include "featurevector.h"


QPicThread::QPicThread(MainWindow *mainWnd)
{
    MainWnd = mainWnd;

    connect(this, SIGNAL(PictureProcessFinish()), MainWnd, SLOT(OnPictureProcessFinish()));
}

void QPicThread::run()
{
    std::vector<std::wstring> path, out;
    for (auto & qs : Path)
        path.push_back(qs.toStdWString());
    GetSubFileList(path, out);

    std::vector<Alisa::ImageInfo> imagesInfo;
    imagesInfo.resize(out.size());

    Alisa::ImageFeatureVector fv;
    fv.Initialize();

    for (size_t i = 0; i < out.size(); ++i)
    {
        QString extension = QString::fromStdWString(out[i].substr(out[i].find_last_of('.') + 1));
        if (extension.compare("png", Qt::CaseInsensitive) &&
            extension.compare("bmp", Qt::CaseInsensitive) &&
            extension.compare("jpg", Qt::CaseInsensitive))
                continue;

        Alisa::Image image;
        if (!image.Open(out[i]))
        {
            Q_ASSERT(0);
            continue;
        }

        imagesInfo[i] = image.GetImageInfo();
        fv.AddPicture(out[i].c_str(), image);
    }

    fv.DivideGroup();

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
            for (size_t k = 0; k < out.size(); ++k)
            {
                if (!out[k].compare(fileName))
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

    emit PictureProcessFinish();

    exit();
}

void QPicThread::SetPath(const QStringList &path)
{
    Path = path;
}
