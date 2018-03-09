#ifndef QPICTHREAD_H
#define QPICTHREAD_H



#include <QThread>

class MainWindow;

class QPicThread : public QThread
{
    Q_OBJECT

public:
    QPicThread(MainWindow *mainWnd);

    virtual void run();

    void SetPath(const QStringList & path);
    bool Abort();

signals:
    void PictureProcessFinish();

private:
    QStringList Path;
    MainWindow *MainWnd;
    bool continueRun;
};



#endif // QPICTHREAD_H
