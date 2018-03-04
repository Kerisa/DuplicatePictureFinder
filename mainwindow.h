#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QThread>
#include <QMainWindow>


namespace Ui {
class MainWindow;
}


class QPicThread;
class QGraphicsScene;
class QTreeWidgetItem;


struct DisplayImage
{
    DisplayImage();
    ~DisplayImage();
    QImage *image{nullptr};
    QGraphicsScene *scene{nullptr};
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void SetGroupResult(std::vector<std::vector<QString>> group) { pictureGroup = group; }

public slots:
    void OnPictureProcessFinish();
    void OnTableItemClicked(bool left, QTreeWidgetItem* item, int colume);

private slots:
    void on_pushButton_clicked();

private:
    void InitMenuBar();
    void InitStatusBar();
    void InitTreeView();

private:
    Ui::MainWindow *ui;
    DisplayImage imageLeft, imageRight;
    QPicThread *procThread;

    std::vector<std::vector<QString>> pictureGroup;
};

class QPicThread : public QThread
{
    Q_OBJECT

public:
    QPicThread(const QString & path, MainWindow *mainWnd);

    virtual void run();

signals:
    void PictureProcessFinish();

private:
    QString Path;
    MainWindow *MainWnd;
};

#endif // MAINWINDOW_H
