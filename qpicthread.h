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

signals:
    void PictureProcessFinish();

private:
    QStringList Path;
    MainWindow *MainWnd;
};



#endif // QPICTHREAD_H
