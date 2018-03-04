#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QThread>
#include <QMainWindow>
#include <QFileInfo>


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

struct TreeViewImageInfo
{
    QString   fileName;
    int       width;
    int       height;
    int       component;
    QFileInfo fileInfo;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void SetGroupResult(const std::vector<std::vector<TreeViewImageInfo>> & group) { pictureGroup = group; }

private:
    void InitMenuBar();
    void InitStatusBar();

public slots:
    void OnPictureProcessFinish();
    void OnTableItemClicked(bool left, QTreeWidgetItem* item, int colume);
    void MenuAct_Save();
    void MenuAct_About();
    void MenuAct_StopSearch();
    void MenuAct_Option();
    void MenuAct_DeleteCheckedFile();

private slots:
    void on_addPath_Btn_clicked();
    void on_removePath_Btn_clicked();
    void on_startSearchBtn_clicked();

private:
    Ui::MainWindow *ui;
    DisplayImage imageLeft, imageRight;
    QPicThread *procThread;

    std::vector<std::vector<TreeViewImageInfo>> pictureGroup;
};

class QPicThread : public QThread
{
    Q_OBJECT

public:
    QPicThread(const QStringList & path, MainWindow *mainWnd);

    virtual void run();

signals:
    void PictureProcessFinish();

private:
    QStringList Path;
    MainWindow *MainWnd;
};

#endif // MAINWINDOW_H
