#ifndef QPICTHREAD_H
#define QPICTHREAD_H


#include <string>
#include <vector>
#include <QThread>
#include <QVector>

namespace Alisa {
class ImageInfo;
class ImageFeatureVector;
}
class MainWindow;
class ReadFileSubThread;
struct SubThreadData;

class QPicThread : public QThread
{
    Q_OBJECT

public:
    QPicThread(MainWindow *mainWnd);

    virtual void run();

    void SetPath(const QStringList & path, float threshold);
    bool Abort();

private:
    int                         GetCPUCoreNum();
    std::vector<std::wstring>   RemoveBadFiles(const std::vector<std::wstring> & in) const;
    std::vector<QString>        StartReadThread(std::vector<std::wstring> & fileGroup, std::vector<Alisa::ImageInfo> & imagesInfo, Alisa::ImageFeatureVector & fv);
    void                        GenerateResult(const Alisa::ImageFeatureVector & fv, const std::vector<std::wstring> & fileGroup, const std::vector<Alisa::ImageInfo> & imagesInfo, const std::vector<QString> & readFailedFile);

signals:
    void PictureProcessFinish();
    void PictureProcessStepMsg(float percent, const QString & msg);

private:
    QStringList Path;
    float Threshold;
    MainWindow *MainWnd;
    bool continueRun;
};



#endif // QPICTHREAD_H
