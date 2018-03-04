#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qlabel.h"
#include "qmessagebox.h"

#include <cassert>
#include <vector>
#include <string>
#include "featurevector.h"
#include "OpenFiles.h"
#include "picture.h"

#include <Windows.h>
#include <QDateTime>
#include <QFileDialog>


DisplayImage::DisplayImage()
    : image(new QImage), scene(new QGraphicsScene)
{
}

DisplayImage::~DisplayImage()
{
    if (image) delete image;
    if (scene) delete scene;
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto b = connect(ui->searchResult_treeWidget, SIGNAL(ClickItem(bool, QTreeWidgetItem*, int)), this, SLOT(OnTableItemClicked(bool, QTreeWidgetItem*, int)));
    assert(b);
    //b = connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(showMouseRightButton(const QPoint)));
    assert(b);

    InitMenuBar();
    InitStatusBar();
    ui->searchResult_treeWidget->InitTreeView();

    procThread = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
    if (procThread) delete procThread;
}

void MainWindow::OnPictureProcessFinish()
{
    ui->searchResult_treeWidget->clear();
    for (size_t i = 0; i < pictureGroup.size(); ++i)
    {
        QStringList content;
        content.push_back("Group<" + QString::number(i + 1) + ">");
        auto top = new QTreeWidgetItem(content);
        ui->searchResult_treeWidget->addTopLevelItem(top);
        top->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        top->setCheckState(0, Qt::PartiallyChecked);
        assert(pictureGroup[i].size() > 1);
        for (size_t f = 0; f < pictureGroup[i].size(); ++f)
        {
            content.clear();
            content.push_back(pictureGroup[i][f].fileName);
            content.push_back(QString::number(pictureGroup[i][f].fileInfo.size()) + "bytes");
            content.push_back(QString::number(pictureGroup[i][f].width) + "x" + QString::number((pictureGroup[i][0].height)));
            content.push_back(pictureGroup[i][f].fileInfo.lastModified().toString("yyyy/MM/dd/ hh:mm:ss"));

            auto entry = new QTreeWidgetItem(content);
            entry->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            entry->setCheckState(0, f == 0 ? Qt::Unchecked : Qt::Checked);
            top->addChild(entry);
        }
    }
    ui->searchResult_treeWidget->expandAll();
    QMessageBox::about(this, "title", "process finished!");
}

void MainWindow::OnTableItemClicked(bool left, QTreeWidgetItem* item, int colume)
{
    assert(item);
    colume;

    auto filename = item->text(0);
    if (left)
    {
        if (imageLeft.image->load(filename))
        {
            imageLeft.scene->clear();
            QRectF ctlSize = ui->leftImg_graphicsView->sceneRect();
            imageLeft.scene->addPixmap(QPixmap::fromImage((*imageLeft.image)).scaled(ui->leftImg_graphicsView->size(), Qt::KeepAspectRatio));

            ui->leftImg_graphicsView->setScene(imageLeft.scene);
            ui->leftImg_graphicsView->show();

            ui->leftImageName_Label->setText(filename);
        }
//        else
//        {
//            QMessageBox::about(this, "title", "load image failed.");
//        }
    }
    else
    {
        if (imageRight.image->load(filename))
        {
            imageRight.scene->clear();
            QRectF ctlSize = ui->rightImg_graphicsView->sceneRect();
            imageRight.scene->addPixmap(QPixmap::fromImage((*imageRight.image)).scaled(ui->rightImg_graphicsView->size(), Qt::KeepAspectRatio));

            ui->rightImg_graphicsView->setScene(imageRight.scene);
            ui->rightImg_graphicsView->show();

            ui->rightImageName_Label->setText(filename);
        }
//        else
//        {
//            QMessageBox::about(this, "title", "load image failed.");
//        }
    }
}

void MainWindow::MenuAct_Save()
{
    QMessageBox::about(this, "title", "save.");
}

void MainWindow::MenuAct_About()
{
    QMessageBox::about(this, "title", "about.");
}

void MainWindow::MenuAct_StopSearch()
{
    QMessageBox::about(this, "title", "StopSearch.");
}

void MainWindow::MenuAct_Option()
{
    QMessageBox::about(this, "title", "Option.");
}

void MainWindow::MenuAct_DeleteCheckedFile()
{
    QMessageBox::about(this, "title", "DeleteCheckedFile.");
}

void MainWindow::InitMenuBar()
{
    QMenuBar *menu_bar = menuBar();

    // 文件菜单
    QMenu *menuFile = new QMenu("文件(&F)", this);
    QAction *action_open = new QAction("打开(&O)", this);
    action_open->setShortcut(QKeySequence(Qt::Key_O));

    QAction *action_new = new QAction("新建(&N)", this);
    action_new->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));

    QAction *action_save = new QAction("保存(&S)", this);
    action_save->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    auto b = connect(action_save, SIGNAL(triggered()), this, SLOT(MenuAct_Save()));
    assert(b);

    menuFile->addAction(action_new);
    menuFile->addAction(action_open);
    menuFile->addAction(action_save);

    menu_bar->addMenu(menuFile);


    // 操作菜单
    QMenu *menuAction = new QMenu("操作(&U)", this);
    QAction *action_deleteCheckedFile = new QAction("删除勾选文件(&D)", this);
    b = connect(action_deleteCheckedFile, SIGNAL(triggered()), this, SLOT(MenuAct_DeleteCheckedFile()));
    assert(b);
    menuAction->addAction(action_deleteCheckedFile);

    menu_bar->addMenu(menuAction);


    // 搜索菜单
    QMenu *menuSearch = new QMenu("搜索(&S)", this);
    QAction *action_startSearch = new QAction("开始(&S)", this);
    b = connect(action_startSearch, SIGNAL(triggered()), this, SLOT(on_startSearchBtn_clicked()));
    assert(b);
    QAction *action_stopSearch = new QAction("停止(&T)", this);
    b = connect(action_stopSearch, SIGNAL(triggered()), this, SLOT(MenuAct_StopSearch()));
    assert(b);
    QAction *action_option = new QAction("选项(&O)", this);
    b = connect(action_option, SIGNAL(triggered()), this, SLOT(MenuAct_Option()));
    assert(b);
    menuSearch->addAction(action_startSearch);
    menuSearch->addAction(action_stopSearch);
    menuSearch->addSeparator();
    menuSearch->addAction(action_option);

    menu_bar->addMenu(menuSearch);


    // 帮助菜单
    QMenu *menuHelp = new QMenu("帮助(&H)", this);
    QAction *action_about = new QAction("关于(&A)", this);
    b = connect(action_about, SIGNAL(triggered()), this, SLOT(MenuAct_About()));
    assert(b);
    menuHelp->addAction(action_about);

    menu_bar->addMenu(menuHelp);
}

void MainWindow::InitStatusBar()
{
    QStatusBar *status_bar = statusBar();

    QLabel *label = new QLabel("label", this);

    //QPushButton *push_btn = new QPushButton("Button", this);

    status_bar->addPermanentWidget(label);

    status_bar->showMessage("Message");
}

void MainWindow::on_startSearchBtn_clicked()
{
    QStringList text;
    for (int i = 0; i < ui->searchPathList->count(); ++i)
    {
        text.push_back(ui->searchPathList->item(i)->text());
    }

    if (procThread)
    {
        QMessageBox::about(this, "title", "wait prve...");
        procThread->wait();
        delete procThread;
        procThread = nullptr;
    }

    if (!procThread)
    {
        procThread = new QPicThread(text, this);
        procThread->start();
    }
}

QPicThread::QPicThread(const QStringList &path, MainWindow *mainWnd)
{
    MainWnd = mainWnd;
    Path = path;

    connect(this, SIGNAL(PictureProcessFinish()), MainWnd, SLOT(OnPictureProcessFinish()));
}

void QPicThread::run()
{
    std::vector<std::wstring> path, out;
    for (auto & qs : Path)
        path.push_back(qs.toStdWString());
    GetSubFileList(path, out);

    Alisa::ImageFeatureVector fv;
    fv.Initialize();

    std::vector<Alisa::Image> images;
    images.resize(out.size());
    for (size_t i = 0; i < out.size(); ++i)
    {
        if (!images[i].Open(out[i]))
        {
            assert(0);
            continue;
        }

        fv.AddPicture(out[i].c_str(), images[i]);
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
                    auto imageBaseInfo = images[k].GetImageInfo();
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
}

void MainWindow::on_addPath_Btn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("浏览目录"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (!dir.isEmpty())
    {
        ui->searchPathList->addItem(dir);
    }
}

void MainWindow::on_removePath_Btn_clicked()
{
    auto row = ui->searchPathList->currentRow();
    delete ui->searchPathList->takeItem((row));
}
