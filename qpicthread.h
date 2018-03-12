﻿#ifndef QPICTHREAD_H
#define QPICTHREAD_H



#include <QThread>

class MainWindow;

class QPicThread : public QThread
{
    Q_OBJECT

public:
    QPicThread(MainWindow *mainWnd);

    virtual void run();

    void SetPath(const QStringList & path, float threshold);
    bool Abort();

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