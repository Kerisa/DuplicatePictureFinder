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
    auto b = connect(ui->treeWidget, SIGNAL(ClickItem(bool, QTreeWidgetItem*, int)), this, SLOT(OnTableItemClicked(bool, QTreeWidgetItem*, int)));
    assert(b);

    InitMenuBar();
    InitStatusBar();
    InitTreeView();

    procThread = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
    if (procThread) delete procThread;
}

void MainWindow::OnPictureProcessFinish()
{
    ui->treeWidget->clear();
    for (size_t i = 0; i < pictureGroup.size(); ++i)
    {
        QStringList content;
        content.push_back(pictureGroup[i][0]);
        auto top = new QTreeWidgetItem(content);
        ui->treeWidget->addTopLevelItem(top);
        for (size_t f = 1; f < pictureGroup[i].size(); ++f)
        {
            content.clear();
            content.push_back(pictureGroup[i][f]);
            top->addChild(new QTreeWidgetItem(content));
        }
    }
    QMessageBox::about(this, "title", "process finished!");
}

void MainWindow::OnTableItemClicked(bool left, QTreeWidgetItem* item, int colume)
{
    assert(item);

    auto filename = item->text(0);
    if (left)
    {
        if (imageLeft.image->load(filename))
        {
            imageLeft.scene->clear();
            QRectF ctlSize = ui->graphicsView->sceneRect();
            imageLeft.scene->addPixmap(QPixmap::fromImage((*imageLeft.image)).scaled(ui->graphicsView->size(), Qt::KeepAspectRatio));

            ui->graphicsView->setScene(imageLeft.scene);
            ui->graphicsView->show();
        }
        else
        {
            QMessageBox::about(this, "title", "load image failed.");
        }
    }
    else
    {
        if (imageRight.image->load(filename))
        {
            imageRight.scene->clear();
            QRectF ctlSize = ui->graphicsView_2->sceneRect();
            imageRight.scene->addPixmap(QPixmap::fromImage((*imageRight.image)).scaled(ui->graphicsView_2->size(), Qt::KeepAspectRatio));

            ui->graphicsView_2->setScene(imageRight.scene);
            ui->graphicsView_2->show();
        }
        else
        {
            QMessageBox::about(this, "title", "load image failed.");
        }
    }
}

void MainWindow::InitMenuBar()
{
    QMenuBar *menu_bar = menuBar();

    QMenu *menu = new QMenu("File(&F)", this);
    QAction *action_open = new QAction("Open(&O)", this);
    action_open->setShortcut(QKeySequence(Qt::Key_O));

    QAction *action_new = new QAction("New(&N)", this);
    action_open->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));

    QAction *action_save = new QAction("Save(&S)", this);
    action_open->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

    menu->addAction(action_new);
    menu->addAction(action_open);
    menu->addAction(action_save);

    menu_bar->addMenu(menu);
}

void MainWindow::InitStatusBar()
{
    QStatusBar *status_bar = statusBar();

    QLabel *label = new QLabel("label", this);

    //QPushButton *push_btn = new QPushButton("Button", this);

    status_bar->addPermanentWidget(label);

    status_bar->showMessage("Message");
}

void MainWindow::InitTreeView()
{
    ui->treeWidget->setColumnCount(4);
    QStringList list;
    list.push_back("Filename");
    list.push_back("Size");
    list.push_back("Solution");
    list.push_back("ModifyTime");
    ui->treeWidget->setHeaderLabels(list);
}

void MainWindow::on_pushButton_clicked()
{
    QString text = ui->lineEdit->text();

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

QPicThread::QPicThread(const QString &path, MainWindow *mainWnd)
{
    MainWnd = mainWnd;
    Path = path;

    connect(this, SIGNAL(PictureProcessFinish()), MainWnd, SLOT(OnPictureProcessFinish()));
}

void QPicThread::run()
{
    std::vector<std::wstring> path, out;
    path.push_back(Path.toStdWString());
    GetSubFileList(path, out);

    Alisa::ImageFeatureVector fv;
    fv.Initialize();

    for (size_t i = 0; i < out.size(); ++i)
    {
        Alisa::Image img;
        if (!img.Open(out[i]))
        {
            assert(0);
            continue;
        }

        fv.AddPicture(out[i].c_str(), img);
    }

    fv.DivideGroup();

    std::vector<std::vector<QString>> groups;
    auto _result = fv.GetGroupResult();
    for (auto & g : _result)
    {
        groups.push_back(std::vector<QString>());
        auto & curGroup = groups.back();
        for (auto & file : g)
        {
            curGroup.push_back(QString::fromStdWString(file.c_str()));
        }
    }
    MainWnd->SetGroupResult(groups);

    emit PictureProcessFinish();
}
