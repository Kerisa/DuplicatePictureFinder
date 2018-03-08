#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qlabel.h"
#include "qmessagebox.h"

#include <vector>
#include <string>
#include "featurevector.h"
#include "OpenFiles.h"
#include "picture.h"

#include <Windows.h>
#include <QDateTime>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QResizeEvent>

#include "movetorecyclebin.h"


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
    Q_ASSERT(b);
    //b = connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(showMouseRightButton(const QPoint)));
    Q_ASSERT(b);

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

void MainWindow::RefreshGraphicImage(const QString &filename, DisplayImage *img, QGraphicsView *view, QLabel *filenameLabel, bool loadFile)
{
    // 图像按控件大小显示时会略微超出显示区域
    auto size = view->size();
    size -= QSize(2, 2);

    if (loadFile)
    {
        if (img->image->load(filename))
        {
            img->scene->clear();
            QRectF ctlSize = view->sceneRect();
            img->scene->addPixmap(QPixmap::fromImage((*img->image)).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            view->setScene(img->scene);
            view->show();

            img->fileName = filename;
            filenameLabel->setText(filename);

            view->setUserData(0, reinterpret_cast<QObjectUserData*>(img));
        }
        else
        {
            //QMessageBox::about(this, "title", "load image failed.");
        }
    }
    else
    {
        // 缩放窗口时不从文件读取
        if (view->userData(0))
        {
            img->scene->clear();
            QRectF ctlSize = view->sceneRect();
            img->scene->addPixmap(QPixmap::fromImage((*img->image)).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            view->setScene(img->scene);
            view->show();
        }
    }
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
        Q_ASSERT(pictureGroup[i].size() > 1);
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
            QFileIconProvider icon;
            entry->setIcon(0, icon.icon(pictureGroup[i][f].fileInfo));
            top->addChild(entry);
        }
    }
    ui->searchResult_treeWidget->expandAll();
    QMessageBox::about(this, "title", "process finished!");
}

void MainWindow::OnTableItemClicked(bool left, QTreeWidgetItem* item, int colume)
{
    Q_ASSERT(item);
    colume;

    auto filename = item->text(0);
    if (left)
    {
        RefreshGraphicImage(filename, &imageLeft, ui->leftImg_graphicsView, ui->leftImageName_Label);
    }
    else
    {
        RefreshGraphicImage(filename, &imageRight, ui->rightImg_graphicsView, ui->rightImageName_Label);
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
    // "确认将所有勾选的文件移除到回收站中？"
    if (QMessageBox::No == QMessageBox::question(this, "cofirm delete", "delete checked files to recycle ?", QMessageBox::Yes, QMessageBox::No))
        return;

    QStringList cannotDeleteFile;
    auto deleteFile = ui->searchResult_treeWidget->GetCheckedFileName();
    for (auto & f : deleteFile)
    {
        if (MoveToRecycleBin::execute(f.fileName))
        {
            ui->searchResult_treeWidget->topLevelItem(f.topLevelIndex)->removeChild(f.childItem);
            delete f.childItem;
        }
        else
        {
            cannotDeleteFile.push_back(f.fileName);
        }
    }

    if (!cannotDeleteFile.empty())
    {
        QString msg("cannot delete listed file:");
        for (auto & f : cannotDeleteFile)
        {
            msg += '\n';
            msg += f;
        }
        msg += "\nplease delete manually.";
        QMessageBox::warning(this, "cannot del file", msg, QMessageBox::Ok);
    }
    else
    {
        QMessageBox::information(this, "delete finish", QString("successful delete %1 file(s)").arg(deleteFile.size()), QMessageBox::Ok);
    }
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
    Q_ASSERT(b);

    menuFile->addAction(action_new);
    menuFile->addAction(action_open);
    menuFile->addAction(action_save);

    menu_bar->addMenu(menuFile);


    // 操作菜单
    QMenu *menuAction = new QMenu("操作(&U)", this);
    QAction *action_deleteCheckedFile = new QAction("删除勾选文件(&D)", this);
    b = connect(action_deleteCheckedFile, SIGNAL(triggered()), this, SLOT(MenuAct_DeleteCheckedFile()));
    Q_ASSERT(b);
    menuAction->addAction(action_deleteCheckedFile);

    menu_bar->addMenu(menuAction);


    // 搜索菜单
    QMenu *menuSearch = new QMenu("搜索(&S)", this);
    QAction *action_startSearch = new QAction("开始(&S)", this);
    b = connect(action_startSearch, SIGNAL(triggered()), this, SLOT(on_startSearchBtn_clicked()));
    Q_ASSERT(b);
    QAction *action_stopSearch = new QAction("停止(&T)", this);
    b = connect(action_stopSearch, SIGNAL(triggered()), this, SLOT(MenuAct_StopSearch()));
    Q_ASSERT(b);
    QAction *action_option = new QAction("选项(&O)", this);
    b = connect(action_option, SIGNAL(triggered()), this, SLOT(MenuAct_Option()));
    Q_ASSERT(b);
    menuSearch->addAction(action_startSearch);
    menuSearch->addAction(action_stopSearch);
    menuSearch->addSeparator();
    menuSearch->addAction(action_option);

    menu_bar->addMenu(menuSearch);


    // 帮助菜单
    QMenu *menuHelp = new QMenu("帮助(&H)", this);
    QAction *action_about = new QAction("关于(&A)", this);
    b = connect(action_about, SIGNAL(triggered()), this, SLOT(MenuAct_About()));
    Q_ASSERT(b);
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

void MainWindow::on_addPath_Btn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("浏览目录"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (dir.isEmpty())
        return;

    // 检查是否重复添加
    auto paths = ui->searchPathList->findItems(dir, Qt::MatchExactly);
    if (!paths.empty())
    {
        QMessageBox::information(this, "same dir", QString("搜索目录中已经存在 %1").arg(dir), QMessageBox::Ok);
        return;
    }

    // 检查是否已经添加了子目录
    paths = ui->searchPathList->findItems(dir, Qt::MatchContains);
    if (!paths.empty())
    {
        // "已经添加了 %1 的子目录，是否将所有子目录移除？"
        if (QMessageBox::Yes == QMessageBox::question(this, "sub dir", QString("remove all sub dir of %1 ?").arg(dir), QMessageBox::Yes, QMessageBox::No))
        {
            for (auto widget = paths.begin(); widget != paths.end(); ++widget)
            {
                ui->searchPathList->removeItemWidget(*widget);
                delete (*widget);
            }
        }
        else
        {
            return;
        }
    }

    // 检查是否添加了父目录
#ifdef _DEBUG
   int parentDirCount = 0;
#endif
    for (int i = 0; i < ui->searchPathList->count(); ++i)
    {
        auto path = ui->searchPathList->item(i)->text();
        if (dir.startsWith(path, Qt::CaseInsensitive))
        {
#ifdef _DEBUG
            ++parentDirCount;
#endif
            if (QMessageBox::Yes == QMessageBox::question(this, "parent dir", QString("已经添加了 %1 的父目录，是否将所有父目录移除并更换为该目录？").arg(dir), QMessageBox::Yes, QMessageBox::No))
            {
                auto widget = ui->searchPathList->takeItem(i);
                delete widget;
#ifndef _DEBUG
                break;
#endif
            }
            else
            {
                return;
            }
        }
    }
#ifdef _DEBUG
    // 至多应当只存在一个父目录
    Q_ASSERT(parentDirCount <= 1);
#endif

    QFileIconProvider icon;
    QListWidgetItem *item = new QListWidgetItem(icon.icon(QFileIconProvider::Folder), dir);
    ui->searchPathList->addItem(item);
}

void MainWindow::on_removePath_Btn_clicked()
{
    auto row = ui->searchPathList->currentRow();
    delete ui->searchPathList->takeItem((row));
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    const int halfWidth = event->size().width() / 2;
    const int halfHeight = event->size().height() / 2;

    //ui->leftImageGroupBox->setGeometry(6, 6, halfWidth - 9, halfHeight - 25);
    //ui->rightImageGroupBox->setGeometry(halfWidth + 3, 6, halfWidth - 9, halfHeight - 25);

    ui->leftImg_graphicsView->setGeometry(6, 6, halfWidth - 9, halfHeight - 25);
    ui->rightImg_graphicsView->setGeometry(halfWidth + 3, 6, halfWidth - 9, halfHeight - 25);

    ui->leftImageName_Label->setGeometry(6, halfHeight - 18, halfWidth - 12, 20);
    ui->rightImageName_Label->setGeometry(halfWidth + 3, halfHeight - 18, halfWidth - 6, 20);

    ui->searchResult_treeWidget->setGeometry(6, halfHeight + 5, halfWidth * 2 - 12, halfHeight * 0.55);

    ui->label->setGeometry(6, halfHeight * 1.57 + 10, 61, 21);
    ui->addPath_Btn->setGeometry(halfWidth * 2 - 6 - 75 - 30 - 30, halfHeight * 1.57 + 10, 23, 23);
    ui->removePath_Btn->setGeometry(halfWidth * 2 - 6 - 75 - 30, halfHeight * 1.57 + 10, 23, 23);
    ui->startSearchBtn->setGeometry(halfWidth * 2 - 6 - 75, halfHeight * 1.57 + 10, 75, 23);

    ui->searchPathList->setGeometry(6, halfHeight * 1.57 + 35, halfWidth * 2 - 12, halfHeight * 0.43 - 95);

    // 调整树列表的列宽
    ui->searchResult_treeWidget->AdjustColumeWidth();

    // 缩放图像
    RefreshGraphicImage(ui->leftImageName_Label->text(), &imageLeft, ui->leftImg_graphicsView, ui->leftImageName_Label, false);
    RefreshGraphicImage(ui->rightImageName_Label->text(), &imageRight, ui->rightImg_graphicsView, ui->rightImageName_Label, false);
}
