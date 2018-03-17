#ifndef QPICTHREAD_H
#define QPICTHREAD_H


#include <string>
#include <vector>
#include <QThread>
#include <QVector>

namespace Alisa {
class ImageInfo;
class ImageFeatureVector;
class FeatureDataRecord;
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
    static int                  GetCPUCoreNum();
    static QString              GetCacheFilePath();
    std::vector<std::wstring>   FilterFiles(const std::vector<std::wstring> & in) const;
    std::vector<QString>        StartReadThread(std::vector<std::wstring> & fileGroup, std::vector<Alisa::ImageInfo> & imagesInfo, Alisa::ImageFeatureVector & fv, Alisa::FeatureDataRecord & fdr);
    void                        GenerateResult(const Alisa::ImageFeatureVector & fv, const std::vector<std::wstring> & fileGroup, const std::vector<Alisa::ImageInfo> & imagesInfo, const std::vector<QString> & readFailedFile) const;

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
